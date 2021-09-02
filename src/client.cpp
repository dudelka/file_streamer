#include "commandline.hpp"
#include "file.hpp"
#include "sender.hpp"
#include "receiver.hpp"
#include "packet_manager.hpp"
#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <utility>
#include <vector>
#include <string>

int main(const int argc, const char* argv[]) {
    try {
        CommandlineParser commandline_args(argc, argv);
        std::vector<File> files = GenerateFiles(*commandline_args.GetFiles());
        std::shared_ptr<Sender> sender = std::make_shared<Sender>(
            commandline_args.GetSendAddress(), commandline_args.GetTimeout()
        );
        Receiver receiver(commandline_args.GetReceiveAddress(), sender, std::move(files));
        receiver.Connect(sender);
        std::thread sender_thread{&Sender::Run, sender.get()};
        receiver.Run();
        // pair id -> checksum
        std::vector<std::pair<uint32_t, uint32_t>> sent_checksums =
            receiver.GetSentChecksums();
        std::vector<std::pair<uint32_t, uint32_t>> received_checksums = 
            receiver.GetReceivedChecksums();
        if (sent_checksums.size() != received_checksums.size()) {
            throw std::runtime_error("Number of sent and received checksums doesn't match.");
        }
        receiver.Shutdown(sender);
        sender->Stop();
        sender_thread.join();
        Sort(sent_checksums);
        Sort(received_checksums);
        uint32_t mismatched = 0;
        for (size_t i = 0; i < sent_checksums.size(); ++i) {
            if (sent_checksums[i].first != received_checksums[i].first) {
                std::string error = 
                    "Found mismatch in files id. Expected to receive checksum for file with id: " 
                    + std::to_string(sent_checksums[i].first) + ", got: " 
                    + std::to_string(received_checksums[i].first) + ".";
                throw std::runtime_error(error);
            }
            std::cout << "Sent file with id = " << std::to_string(sent_checksums[i].first) 
                << " and checksum = " << std::to_string(sent_checksums[i].second) << ". Got: " 
                << std::to_string(received_checksums[i].second) << "." << std::endl;
            if (sent_checksums[i].second == received_checksums[i].second) {
                std::cout << "Checksums matches." << std::endl;
            } else {
                std::cout << "Checksums doesn't match." << std::endl;
                mismatched++;
            }
            std::cout << std::endl;
        }
        if (mismatched != 0) {
            std::cout << "Mismatched checknums " << mismatched << " times." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
