#include "Client.h"

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>\n";
            return 1;
        }

        asio::io_context io_context;
        tcp::resolver r(io_context);
        Client c(io_context);

        c.Start(r.resolve(argv[1], argv[2]));

        std::thread t([&io_context]() {
            io_context.run();
        });

        char line[kMaxBodyLength + 1];
        while (std::cin.getline(line, kMaxBodyLength + 1) && !c.IsClosed()) {
            ChatMessage msg;
            msg.BodyLength(std::strlen(line));
            std::memcpy(msg.Body(), line, msg.BodyLength());
            msg.EncodeHeader();
            c.Write(msg);
        }

        c.Stop();
        t.join();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}