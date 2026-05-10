My GitHub repository link: https://github.com/RollsIllusion/OOP-Project

# PROJECT REPORT

# MediCore Hospital Management System

##### This project report presents the development of MediCore, a comprehensive Hospital Management System developed as part of the Object Oriented Programming course during Spring 2026. The system is designed to efficiently manage core hospital operations including patient records, doctor information, appointment scheduling, medical prescriptions, billing, and administrative tasks. It supports three distinct user roles — Patient, Doctor, and Admin — each with appropriate functionalities and secure access controls. The entire application was developed following strict project guidelines, emphasizing proper object-oriented design principles while adhering to all specified constraints.

##### The MediCore system demonstrates strong application of OOP concepts through a well-structured class hierarchy. The abstract base class Person is inherited by Patient, Doctor, and Admin classes, effectively utilizing inheritance and polymorphism. Other key classes include Appointment, Bill, Prescription, and a generic Storage<T> template class that manages collections of up to 100 records without using std::vector. All data is persistently stored in text files (.txt) and loaded into memory at startup. The FileHandler class exclusively manages all file operations, ensuring immediate persistence after every modification. Input validation is centralized in the Validator class, while custom exceptions handle various error scenarios such as insufficient funds, slot unavailability, and invalid inputs.

##### Key features of the system include secure login mechanisms with three-attempt lockout and security logging, appointment booking with specialization-based doctor search and real-time slot availability checking, cancellation with automatic refund, medical record viewing, bill management and payment, wallet top-up functionality, doctor appointment management, and comprehensive admin controls including doctor management, patient discharge with archiving, and daily report generation. The system ensures no global variables are used, all data is user-provided through files, and banned library functions are avoided through manual implementations of string and numeric operations.

##### A notable enhancement in this project is the development of both console-based and graphical user interfaces. The console interface is implemented in Menu.cpp, while a complete SFML-based graphical user interface has been developed in gui.h and gui.cpp, offering buttons, input fields, scrollable lists, and modern visual feedback. The DateTime class encapsulates all date and time operations, ensuring clean separation of concerns. Special attention was given to code readability, proper commenting, modular design, and memory leak prevention as per the project requirements.

##### The project was tested extensively for all major workflows, edge cases, validation rules, exception handling, and data consistency across program restarts. All functionalities work as expected, with proper handling of scenarios such as slot conflicts, insufficient balance, overdue bills, and discharge validations. The codebase strictly follows the submission instructions, including individual .h and .cpp files, proper GitHub integration (repository link would be submitted with the zip), and comprehensive README.md.

##### In conclusion, the MediCore Hospital Management System successfully fulfills all the requirements of the OOP project. It showcases effective use of object-oriented programming principles, robust error handling, file management, and user-friendly design. This project has significantly enhanced my understanding of class design, template programming, exception handling, and building complete applications under strict constraints. The system is fully functional, maintainable, and ready for submission.

##### Submitted by: Musa Bin Amjad

##### Roll Number: 25L-0925

##### Section: BCS-2B






