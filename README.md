# GoFlash

**GoFlash** is an ultra-compact device based on **ESP32-S3** that looks like a regular USB flash drive but enables interaction with **AI from an offline computer**.

To the computer it appears as a normal **USB storage device**, while internally the device:

* connects to Wi-Fi
* sends requests to an AI model
* writes the response back to a file

All networking and AI communication happens **inside the microcontroller**, meaning the host computer performs **no network activity at all**.

# Concept

In many environments (for example during exams or lab tests):

* computers **have no internet access**
* phones are **not allowed**
* installing software is **restricted**

GoFlash approaches this differently:

* the computer only interacts with a **text file**
* the device behaves like a **regular flash drive**
* all AI communication happens **on the device itself**

Internet access is provided via a **Wi-Fi hotspot**.

# User Workflow

1. Plug GoFlash into a computer
2. Open the file

```
DATA.TXT
```

3. Write a prompt or question
4. Save the file
5. Create a trigger file

```
GO.TXT
```

After a few seconds the AI response appears inside `DATA.TXT`.

# Key Features

* USB Mass Storage emulation (appears as a flash drive)
* RAM-based virtual disk
* AI integration via **Google Gemini**
* Wi-Fi connectivity through hotspot
* minimal traces on the host system
* extremely small hardware footprint

# Architecture

GoFlash combines several independent components:

* **USB MSC emulation**
  allows the device to appear as a standard storage drive

* **RAM-based FAT filesystem**
  a minimal virtual disk stored entirely in memory

* **Wi-Fi networking**
  used for internet access

* **AI client**
  responsible for communication with the language model

# How It Works

When a trigger file appears on the virtual disk, the device:

1. temporarily disconnects the USB storage
2. reads the user prompt from the data file
3. sends it to the AI model
4. writes the response back to the file
5. reconnects the storage device

From the computer’s perspective this only looks like a brief USB reconnection.

# Hardware

The project is designed for **ESP32-S3 boards** with USB OTG support.

Recommended board:

* ESP32-S3 Super Mini

Due to the small form factor, the device can easily be disguised as:

* a USB flash drive
* a USB adapter
* a small USB dongle

# Internet Connectivity

The device uses **Wi-Fi** to reach the AI API.

The most convenient option is a **mobile hotspot**.

This means:

* the computer itself never connects to the internet
* no network requests originate from the host
* all external communication is handled by the ESP32

# Technologies

The project is built with:

* **ESP32-S3**
* **Arduino framework**
* **USB Mass Storage**
* **FAT filesystem**
* **Google Gemini API**
* **PlatformIO**

# Setup

1. Clone the repository

```
git clone https://github.com/danio273/go-flash
```

2. Open the project in **PlatformIO**

3. Configure:

* Wi-Fi credentials
* API key

4. Build and flash the firmware.

# Limitations

* response size is limited by the RAM disk
* only one active request at a time
* requires Wi-Fi connectivity

# Possible Improvements

* multiple request files
* encryption
* automatic trigger detection

# Disclaimer

This project was created for experimentation and learning in areas such as:

* embedded systems
* USB device development
* lightweight filesystems
* integrating AI with microcontrollers

Use responsibly.