import Foundation
import Network

let port = NWEndpoint.Port(rawValue: 5000)!

let listener = try NWListener(using: .udp, on: port)

listener.stateUpdateHandler = { state in
    switch state {
    case .ready:
        print("PSP-ICE UDP receiver listening on port 5000")
    case .failed(let error):
        print("Listener failed: \(error)")
    default:
        break
    }
}

listener.newConnectionHandler = { connection in

    connection.stateUpdateHandler = { state in
        if case .ready = state {
            print("Sender connected!")
            receiveMessage(from: connection)
        }
    }

    connection.start(queue: .main)
}

func receiveMessage(from connection: NWConnection) {

    connection.receiveMessage { data, context, isComplete, error in

        if let data = data,
           let message = String(data: data, encoding: .utf8) {
            print("Received: \(message)")
        }

        if let error = error {
            print("Receive error: \(error)")
            return
        }

        receiveMessage(from: connection)
    }
}

listener.start(queue: .main)

dispatchMain()