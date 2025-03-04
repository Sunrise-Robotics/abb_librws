#include <iostream>
#include <string>
#include <abb_librws/v2_0/rws_client.h>
#include <abb_librws/v2_0/rws_interface.h>
#include <abb_librws/v2_0/rw/rapid.h>

int main() {
    std::string ip_address = "192.168.1.101";  // Default IP address
    unsigned short port = 5466;                // Default port
    std::string input;

    // Prompt the user for the IP address and port with defaults
    std::cout << "Enter the IP address of the RWS server [" << ip_address << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        ip_address = input;
    }

    std::cout << "Enter the port of the RWS server [" << port << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            port = static_cast<unsigned short>(std::stoi(input));
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number, using default: " << port << std::endl;
        }
    }

    std::cout << "Connecting to " << ip_address << ":" << port << std::endl;

    try {
        // Set up connection options
        abb::rws::ConnectionOptions connection_options(ip_address, port, "Default User", "robotics");

        // Create an RWS client
        abb::rws::v2_0::RWSClient client(connection_options);

        // Create an RWS interface
        abb::rws::v2_0::RWSInterface interface(client);

        // Get RAPID execution information
        abb::rws::rw::RAPIDExecutionInfo execution_info = abb::rws::v2_0::rw::rapid::getRAPIDExecution(client);

        // Get the Joint Target
        abb::rws::v2_0::RWSClient::RWSResult joint_target = client.getMechanicalUnitJointTarget("ROB_1");

        // Output the execution state and run mode
        // std::cout << "RAPID Execution State: " << (execution_info.ctrlexecstate == abb::rws::v2_0::rw::rapid::RAPIDExecutionState::running ? "Running" : "Stopped") << std::endl;
        std::cout << "RAPID Run Mode: " << static_cast<int>(execution_info.cycle) << std::endl;
        std::cout << "Joint Target: " << joint_target << std::endl;  // Use getRawData() to get a string representation

        // Check if the motors are on
        if (interface.isMotorsOn()) {
            std::cout << "Motors are on. Turning them off before proceeding." << std::endl;
            interface.setMotorsOff();
        } else {
            std::cout << "Motors are off" << std::endl;
        }

        // Set the motors on
        interface.setMotorsOn();

        // Check if the motors are on
        if (interface.isMotorsOn()) {
            std::cout << "Motors are on" << std::endl;
        }

        // Turn them back off
        interface.setMotorsOff();

        // Check if the motors are on
        if (interface.isMotorsOn()) {
            std::cout << "Motors are on" << std::endl;
        } else {
            std::cout << "Motors are off" << std::endl;
        }


    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}