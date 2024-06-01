# Indian Citizen Database using Nested B+ Trees

This project implements an Indian Citizen database using nested B+ trees from scratch in C. The database is structured such that each leaf contains the Aadhar data of an individual. Each leaf also contains a pointer to another B+ tree whose leaves contain all the PANs of that person. Furthermore, each PAN leaf contains a pointer to another B+ tree whose leaves are the banks associated with the corresponding PAN card. The insertion process is optimized for both efficiency and space economy, utilizing a parent stack for back propagation during splits. 

## Features

This database implementation supports the following functionalities:

1. **Print citizens without PAN**: Lists all citizens who do not have a PAN card.
2. **Print citizens with multiple PAN cards**: Lists all citizens who have multiple PAN cards.
3. **Print citizens with multiple Bank accounts**: Lists all citizens who have multiple bank accounts.
4. **Print citizens with LPG subsidy**: Lists all citizens who receive an LPG subsidy.
5. **Print citizens with total threshold balance**: Lists all citizens whose total bank balance exceeds a specified threshold.
6. **Print citizens with inconsistent data**: Identifies citizens with anomalies in their information across different identity cards like bank, PAN, or Aadhar cards.
7. **Add New Bank dataset**: Integrates a new bank database into the existing database.

## Goodness of Using Nested B+ Trees

### Efficient Range Search
Using B+ trees provides efficient range search capabilities with predictable search times. This is particularly beneficial for operations that need to retrieve data within a certain range, such as listing citizens whose total balance exceeds a threshold.

### Space Efficiency
The insertion process is optimized for space economy, ensuring that the database remains compact and efficient even as it grows. The parent stack and back propagation mechanism help manage splits efficiently, maintaining balanced trees.

### Hierarchical Organization
The hierarchical structure of the database (Aadhar -> PAN -> Bank) allows for efficient data management and retrieval. It ensures that related data is closely linked, making operations like identifying citizens with multiple PAN cards or bank accounts straightforward and efficient.

### Consistency and Anomaly Detection
The `Print citizens with inconsistent data` function leverages the hierarchical structure to cross-verify information across different identity cards, making it easier to detect and resolve inconsistencies.

### Flexibility in Data Integration
The `Add New Bank dataset` function demonstrates the flexibility of the nested B+ tree structure in integrating new data sources. This adaptability is crucial for maintaining and updating the database as new data becomes available.

## Getting Started

### Prerequisites
- C Compiler (GCC recommended)

### Compilation
To compile the project, use the following command:
```sh
gcc -o citizen_db main.c bptree.c aadhar.c pan.c bank.c
