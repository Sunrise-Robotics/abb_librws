#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <abb_librws/v2_0/rws_client.h>
#include <abb_librws/v2_0/rws_interface.h>
#include <abb_librws/v2_0/rw/rapid.h>
#include <abb_librws/rws_state_machine_interface.h>

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

        // Create the state machine interface
        abb::rws::RWSStateMachineInterface state_machine_interface(client);

        // Get RAPID execution information
        abb::rws::rw::RAPIDExecutionInfo execution_info = abb::rws::v2_0::rw::rapid::getRAPIDExecution(client);

        // Get the Joint Target
        abb::rws::v2_0::RWSClient::RWSResult joint_target = client.getMechanicalUnitJointTarget("ROB_1");

        // Declare the MastershipDomain enum
        abb::rws::v2_0::MastershipDomain mastership_domain = abb::rws::v2_0::MastershipDomain::edit;

        // Output the execution state and run mode
        // std::cout << "RAPID Execution State: " << (execution_info.ctrlexecstate == abb::rws::v2_0::rw::rapid::RAPIDExecutionState::running ? "Running" : "Stopped") << std::endl;
        std::cout << "RAPID Run Mode: " << static_cast<int>(execution_info.cycle) << std::endl;
        std::cout << "Joint Target: " << joint_target << std::endl;  // Use getRawData() to get a string representation

        // Check if Auto mode is enabled
        if (interface.isAutoMode()) {
            std::cout << "Auto mode is enabled" << std::endl;
        } else {
            std::cout << "Auto mode is disabled. Exiting program." << std::endl;
            return 0;
        }

        // Check if the motors are on
        if (interface.isMotorsOn()) {
            std::cout << "Motors are on. Turning them off before proceeding." << std::endl;
            interface.setMotorsOff();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // Wait for the motors to turn off or a maximum of 1 second
            // Wait for motors to turn off with a timeout of 1 second
            auto start_time = std::chrono::steady_clock::now();
            auto end_time = start_time + std::chrono::seconds(1);

            // Loop at approximately 100 Hz until motors are off or timeout
            while (interface.isMotorsOn()) {
                // Check if we've exceeded the timeout
                if (std::chrono::steady_clock::now() >= end_time) {
                    std::cout << "Timeout waiting for motors to turn off" << std::endl;
                    break;
                }

                // Sleep for 10ms (100 Hz frequency)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            std::cout << "Motors are now off or timeout occurred" << std::endl;
        } else {
            std::cout << "Motors are off" << std::endl;
        }

        // Set the motors on
        bool motors_on_success = false;
        int retry_count = 0;
        const int max_retries = 5;

        while (!motors_on_success && retry_count < max_retries) {
            try {
                std::cout << "Attempting to turn motors on (attempt " << retry_count + 1 << " of " << max_retries << ")..." << std::endl;
                interface.setMotorsOn();
                motors_on_success = true;
                std::cout << "Motors turned on successfully" << std::endl;
            }
            catch (const std::exception& e) {
                retry_count++;
                std::cerr << "Failed to turn motors on: " << e.what() << std::endl;

                if (retry_count < max_retries) {
                    std::cout << "Retrying in 1 second..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                else {
                    std::cerr << "Maximum retry attempts reached. Could not turn motors on." << std::endl;
                }
            }
        }
        std::cout << "Turned motors on after " << retry_count << " attempts" << std::endl;

        // Check if the motors are on
        if (interface.isMotorsOn()) {
            std::cout << "Motors are on" << std::endl;
        }

        // Request mastership
        interface.requestMastership(abb::rws::v2_0::MastershipDomain::edit);

        // Set program pointer to MAIN
        interface.resetRAPIDProgramPointer();

        // Start the program
        interface.startRAPIDExecution();

        // Wait a little bit
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Stop the program
        interface.stopRAPIDExecution();

        // Release mastership
        interface.releaseMastership(abb::rws::v2_0::MastershipDomain::edit);

        // Clean-up procedure
        try {
            // Check if the motors are on
            if (interface.isMotorsOn()) {
                std::cout << "Motors are on. Turning them off." << std::endl;
                // Turn them back off
                interface.setMotorsOff();
            } else {
                std::cout << "Motors are off" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "An error occurred: " << e.what() << std::endl;

            // Cast to boost::exception to get detailed diagnostic information
            try {
                if (const boost::exception* be = dynamic_cast<const boost::exception*>(&e)) {
                    std::cerr << "Detailed error info:\n" << boost::diagnostic_information(*be) << std::endl;
                }
            } catch (...) {
                std::cerr << "Failed to extract detailed error information" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;

        // Cast to boost::exception to get detailed diagnostic information
        try {
            if (const boost::exception* be = dynamic_cast<const boost::exception*>(&e)) {
                std::cerr << "Detailed error info:\n" << boost::diagnostic_information(*be) << std::endl;
            }
        } catch (...) {
            std::cerr << "Failed to extract detailed error information" << std::endl;
        }
    }



    return 0;
}