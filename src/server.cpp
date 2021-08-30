#include "commandline.hpp"
#include "sender.hpp"
#include "receiver.hpp"

#include <iostream>

int main(const int argc, const char* argv[]) {
    try {
        CommandlineParser commandline_parser(argc, argv);
        std::shared_ptr<Sender> sender = 
            std::make_shared<Sender>(commandline_parser.GetSendAddress());
        Receiver receiver(commandline_parser.GetReceiveAddress(), 
            sender, commandline_parser.GetTimeout());
        receiver.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
