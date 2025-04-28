# Gym Management System

A C++ based gym management system with user authentication and member management capabilities.

## Features

- User authentication (Admin and Member login)
- Member management
- Customer database using SQLite
- Secure password handling
- User-friendly interface

## Prerequisites

- C++ compiler (g++ or MSVC)
- SQLite3
- Windows OS (for password masking functionality)

## Dependencies

The project uses the following main components:
- SQLite3 for database management
- Standard C++ libraries
- Windows-specific libraries (conio.h for password masking)

## Project Structure

```
CPPProject/
├── headers/
│   ├── interface.hpp
│   ├── member_interface.hpp
│   ├── user_table.h
│   ├── sha256.h
│   └── ... (other header files)
├── main.cpp
└── README.md
```

## Setting Up Required Files

1. Download SQLite:
    -download this linke `https://sqlite.org/2025/sqlite-amalgamation-3490100.zip`
     - (contains sqlite3.c and sqlite3.h)
   - Extract the files to your project directory

2. Set up the project:
   - Place `sqlite3.c` in your project root directory
   - Place `sqlite3.h` in your project root directory 

3. Verify your directory structure:
```
CPPProject/
├── headers/
│   ├── interface.hpp
│   ├── member_interface.hpp
│   ├── user_table.h
│   ├── sha256.h
│   └── ... (other header files)
├── sqlite3.c
├── sqlite3.h
├── main.cpp
└── README.md
```

## Building the Project

1. Make sure you have a C++ compiler installed
2. Install SQLite3 development libraries
3. Compile the project using:

```bash
gcc -c sqlite3.c -o sqlite3.o
g++ main.cpp headers/sha256.cpp sqlite3.o -o crm_program -I./headers
```

## Running the Application

1. After compilation, run the executable:
```bash
./crm_program
```

2. Default admin credentials:
   - Username: admin
   - Password: adminpass

## Usage

1. Launch the application
2. Choose between admin or member login
3. For admin access:
   - Use the admin credentials
   - Access full management features
4. For member access:
   - Use your registered username
   - Use your phone number as password

## Notes

- The system automatically creates user accounts for existing customers
- Passwords are masked during input for security
- The database file (test.db) will be created automatically on first run

## Security Features

- Password masking during input
- SHA-256 hashing for password storage
- Input validation for usernames
- Secure session management

## Error Handling

The application includes comprehensive error handling for:
- Invalid login attempts
- Database connection issues
- Input validation
- General runtime errors

## Contributing

Feel free to submit issues and enhancement requests.