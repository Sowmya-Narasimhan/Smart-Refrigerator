---
title: "Smart Refrigerator System using RFID"
author: "Sowmya N"
date: "`r Sys.Date()`"
output:
  html_document:
    theme: united
    toc: true
    toc_float: true
    number_sections: true
---

# Project Overview

The Smart Refrigerator System using RFID is designed to automate the process of monitoring food items stored inside a refrigerator. Traditional refrigerators require manual tracking of stored items and expiration dates, which can lead to food wastage or forgetting stored products.

This project uses RFID technology to identify and track food items placed inside the refrigerator. Each item is attached with an RFID tag, and when scanned by the RFID reader, the system updates the inventory automatically. The system helps users monitor stored food items efficiently and improves food management.

---

# Objectives

* To automate food inventory tracking inside a refrigerator.
* To reduce food wastage by monitoring stored items.
* To provide a simple and efficient system using RFID technology.
* To demonstrate the use of IoT and embedded systems in smart home applications.

---

# Technologies Used
  * ESP32: Microcontroller for system control
  * RFID RC522 Module:Identification of tagged items
  * RFID Tags: Unique identification for food items
  * Blynk app: Displaying item details on app and website
  * Arduino IDE: Programming and system logic
  
---

# System Architecture

The system consists of an RFID reader connected to a microcontroller. Each food item is associated with an RFID tag. When an item is placed near the RFID reader, the system reads the tag ID and checks the stored database of items.

Based on the tag information, the refrigerator inventory is updated and the item details are displayed on the Blynk app.

System workflow:

1. RFID tag is attached to food item.
2. RFID reader scans the tag when the item is placed near the reader.
3. ESP32 reads the tag ID.
4. The system checks the item information stored in the program.
5. Item details are displayed on the Blynk platform.
6. Inventory is updated accordingly.

---

# Implementation

## Hardware Components

* Esp32
* RFID RC522 Reader
* RFID Tags
* Jumper wires
* Breadboard
* Power supply

## Working Principle

The RFID reader continuously scans for RFID tags placed within its range. When a tagged item is detected, the reader retrieves the tag's unique identification number. The microcontroller compares this ID with predefined IDs stored in the program.

If a match is found, the corresponding food item information is displayed on the Blynk platform.  This allows users to identify and track items stored in the refrigerator.

---

# Results

The system successfully identifies RFID tags attached to food items and displays the item information on the LCD screen. The RFID-based system simplifies inventory tracking and reduces manual effort.

The project demonstrates how embedded systems and RFID technology can be integrated to develop smart home solutions.

---






