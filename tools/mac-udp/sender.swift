import Foundation
import Network

let host = NWEndpoint.Host("127.0.0.1")
let port = NWEndpoint.Port(rawValue: 5000)!

let connection = NWConnection(
    host: host,
    port: port,
    using: .udp
)

connection.stateUpdateHandler = { state in
    if case .ready = state {
        print("Connected to PSP-ICE receiver!")

        let message = "HELLO FROM SWIFT"

        connection.send(
            content: message.data(using: .utf8),
            completion: .contentProcessed { error in
                if let error = error {
                    print("Send failed: \(error)")
                } else {
                    print("Message sent!")
                }

                connection.cancel()
                exit(0)
            }
        )
    }
}

connection.start(queue: .main)
dispatchMain()