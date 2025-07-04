# 🛒 Market Management System in C

This project is a market management system developed in the C programming language, using data structures such as Binary Search Trees (BST), Linked Lists, and QuickSort sorting. It was created as part of an academic project focused on file manipulation, efficient data organization, and simulation of a real-world sales environment.

---

## 📚 Features

- 📦 **Product registration** with categories: Cleaning, Food, and Bakery
- 🛍️ **Shopping cart** with quantity control and automatic total calculation
- 🧾 **Sale finalization** with payment option via cash or card
- 📂 **File persistence** (`produtos.txt`, `caixa.txt`, `vendas_detalhadas.txt`, `log.txt`)
- 🌳 **Binary Search Tree (BST)** for storing and searching products
- 📋 **Product listing ordered by code or name** (using `qsort`)
- 📈 **Detailed sales reports**
- 💾 **Cash register control** with open/close functionality and balance saving
- 🧹 **Automatic stock reset for the bakery** when closing the register

---

## 🧠 Technologies and Concepts Used

- C Language (C99 standard)
- Binary Search Trees (BST)
- Linked Lists
- Dynamic memory allocation using `malloc` and `free`
- Sorting with `qsort`
- File handling (`fopen`, `fclose`, `fscanf`, `fprintf`)
- Date/time formatting using `strftime`
- System event logging
- Terminal interface with `system("cls")`, `system("pause")`, and `Sleep()`

---

## 🗃️ File Structure

|--------------------------|---------------------------------------------|
| File                     | Purpose                                     |
|--------------------------|---------------------------------------------|
| `produtos.txt`           | Stores all persistent products              |
| `caixa.txt`              | Saves the cash register balance between runs|
| `vendas_detalhadas.txt`  | Logs each product sold in detail            |
| `log.txt`                | Logs system events and errors               |
|--------------------------|---------------------------------------------|

---

## 🚀 How to Run

1. Compile the code:

```bash
gcc -o market berenicerealoficial4khd_comentado.c
