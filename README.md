<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a name="readme-top"></a>
<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->

<!-- PROJECT HEADER -->
<br />
<div align="center">

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

<h3 align="center">Zynq-7000 Based TFTP Server v1.1.0</h3>

  <p align="center">
    This project encompasses a TFTP (Trivial File Transfer Protocol) server operating on a card equipped with Xilinx's Zynq-7000 series FPGA and ARM processor combination.
    <br /> <br />
    <a href="https://github.com/efetunca/Zynq-7000-TFTP-Server/issues">Report Bug</a>
    or
    <a href="https://github.com/efetunca/Zynq-7000-TFTP-Server/issues">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary><b>Table of Contents</b></summary>
  <ol>
    <li><a href="#about-the-project">About The Project</a></li>
    <li>
      <a href="#getting-started">Getting Started</a>
        <ul>
          <li><a href="#prerequisites">Prerequisites</a></li>
          <li><a href="#importing-project-to-vitis">Importing Project to Vitis</a></li>
      </ul>
    </li>
    <li>
      <a href="#usage">Usage</a>
        <ul>
          <li><a href="#flashing-the-software-to-the-qspi">Flashing the Software to the QSPI (deprecated)</a></li>
          <li><a href="#connecting-to-the-server">Connecting to the Server</a></li>
          <li><a href="#showing-the-outputs-of-the-board">Showing the Outputs of the Board</a></li>
          <li><a href="#automatic-flashing">Automatic Flashing</a></li>
        </ul>
    </li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project
This project presents a specialized implementation of a TFTP (Trivial File Transfer Protocol) server designed for the Zynq-7000 SoC, specifically utilizing the AMD Zynq 7000 SoC ZC702 Evaluation Kit. The focus of this project is to harness the capabilities of the ZC702 board, transforming it into a dedicated server that facilitates efficient file transfers. Users can directly transfer files to the SD card located on the ZC702 evaluation board via an Ethernet connection. This setup not only ensures high-speed data transmission but also maintains simplicity and ease of use. The project is tailored to cater to environments where quick and efficient file transfer is crucial, such as in embedded systems development and educational applications related to the Zynq-7000 SoC. By concentrating on the TFTP protocol and the specific features of the ZC702 board, the project aims to deliver a robust and user-friendly file transfer solution that leverages the full potential of the Zynq-7000 platform.

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started
This guide will help you set up and start using the Zynq-7000 TFTP server. Before beginning, ensure you have the AMD Zynq 7000 SoC ZC702 Evaluation Kit and an Ethernet connection. If you have are facing any issues in this section, feel free to contact me! 

### Prerequisites
There is no specific pre-requisites list for this project. You just need to have:

* Xilinx Vitis for programming (I'm using 2020.2 version)
* A TFTP application to send and receive files (I'm using Tftpd64 by Philippe Jounin)
* Appropriate drivers for using the UART with ZC702 Evaluation Kit (CP210x USB to UART Bridge driver needed to view `printf` statements. You can click <a href="https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads">here</a> to go to the download page of this driver.)

### Importing Project to Vitis

1. Download `TFTP_server_v1.1.0.zip` file from <a href="https://github.com/efetunca/Zynq-7000-TFTP-Server/releases/tag/1.1.0">releases</a> page.
2. Create a new folder for the workspace.
3. Open Vitis and select the newly created workspace folder.
4. Select "Vitis project exported zip file" and click "Next.
6. Click "Browse..." button next to the "Archive File" text and select the `TFTP_server_v1.1.0.zip` file.
7. Make sure all of these 3 projects are ticked:
  * TFTP_server-app
  * TFTP_server-app_system
  * TFTP_server-platform
  If any of them are not ticked, tick it.
8. Click "Finish" button. After importing is finished, right click on the `TFTP_server-app_system` project and click "Clean Project". After you see thee "Build Finished" text in the console below, right click again the `TFTP_server-platform` project and click "Build Project". And then wait until the "Build Finished" text to appear on the console. Then build the `TFTP_server-app_system` project, either.

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- USAGE EXAMPLES -->
## Usage
You have various options to run this software on the ZC702 board:
* JTAG
* QSPI
* SD card etc.

The only methods I tried were JTAG and QSPI. To use this program with JTAG, you just need to debug the project after importing it into Vitis. However, you will only be able to use this program when connected to the computer via a USB cable. As a second and recommended method, you can connect it to the computer via JTAG and write the software to the QSPI. In this way, after turning off the board, setting the boot switch as necessary and turning it on again, you will see the software running automatically.

### ~~Flashing the Software to the QSPI~~
~~**You must build the project before these steps.**~~

**(DEPRECATED, SEE <a href="#automatic-flashing">AUTOMATIC FLASHING</a> SECTION)**

1. Right click on the `TFTP_server-app_system` and select "Create Boot Image".
2. Click "Browse" button next to the "Output Path" text and select a path for the output file and name it as **BOOT.BIN**. Be aware, you should click the button next to the "Output Path" text, not "Output BIF File Path".
3. Click "Create Image" button. If any message box appears that says a BIF file already exists, just click OK and wait for the "Bootimage generated successfully" text in the console down below. 
4. Right click on the `TFTP_server-app_system` and select "Program Flash".
5. Click "Browse" button next to the "Image File" text and find the `BOOT.BIN` file you've just created.
6. Click "Program" button and wait for flashing to finish. Be aware, do not turn off the board while flashing software!

### Connecting to the Server
In order to use this server, you need to make your computer's Ethernet settings as in the software. The necessary IP address settings for the board are located in the `tftp_server.h` file. You can change these settings as you wish. If you want to connect to the server with the default settings:

1. Go to your computer's ethernet settings and set your computer's IPv4 address to `192.168.1.X`. Since the board's default IPv4 address is `192.168.1.10`, you need to select another number instead of X.
2. You need to set your computer's IPv4 mask to `255.255.255.0`. If you want to use a different setting, you must change it in the software.
3. There is no setting you need to enter as Gateway, but you can enter it as `192.168.1.1`.
4. You do not need to define any DNS settings.

After doing these settings, run your favorite TFTP program. Enter `192.168.1.10` as the IP address (default value, can be changed) and `69` as the port number and then connect to the server. Afterwards, you can transfer files to the SD card installed on the board or transfer the files on this card to the computer.

After each read and write request transmitted to the board, the `index.html` file automatically created by the software on the SD card will be automatically updated. This file contains a file tree that shows the files and folders on the SD card.

### Showing the Outputs of the Board
You can see the outputs of the software from the moment it runs through a program such as PuTTY. For this, you must download and install the driver given in the <a href="#prerequisites">Prerequisites</a> section. Then, you need to run PuTTY or another program of your choice, select the COM port which the board is connected to and set the baudrate to 115200.

### Automatic Flashing
You can now automatically flash a `BOOT.BIN` file by just sending the file over TFTP. The program automatically recognizes the file and does the necessary QSPI Flashing operations. You just have to monitor the progress coming through the UART on a terminal like PuTTY, and turn off the board and turn it on again in QSPI boot mode when the operations are completed.

> If you want to use a different boot file name, you have to change the `BOOT_FILE_NAME` define in the `web_utils.h` file.

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

- [x] Automatically flashing the BOOT.BIN file to QSPI.
    - [ ] Store the BOOT.BIN file encrypted.

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue.

**Don't forget to give the project a star! Thanks again!**

1. Fork the Project
2. Create your Feature Branch
```sh
git checkout -b feature
```
3. Commit your Changes
```sh
git commit -m 'Added some Feature'
```
4. Push to the Branch
```sh
git push origin feature
```
5. Open a Pull Request

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- LICENSE -->
## License
Distributed under the MIT License. See <a href="https://github.com/efetunca/Zynq-7000-TFTP-Server/blob/main/LICENSE">`LICENSE`</a> for more information.

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- CONTACT -->
## Contact

If you have any questions regarding this project, please feel free to reach out to me at efetunca@hotmail.com.

<p align="right">(<a href="#readme-top">Back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[stars-shield]: https://img.shields.io/github/stars/efetunca/Zynq-7000-TFTP-Server?style=for-the-badge
[stars-url]: https://github.com/efetunca/Zynq-7000-TFTP-Server/stargazers
[issues-shield]: https://img.shields.io/github/issues/efetunca/Zynq-7000-TFTP-Server?style=for-the-badge
[issues-url]: https://github.com/efetunca/Zynq-7000-TFTP-Server/issues
[license-shield]: https://img.shields.io/github/license/efetunca/Zynq-7000-TFTP-Server?style=for-the-badge
[license-url]: https://github.com/efetunca/Zynq-7000-TFTP-Server/blob/main/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/efetunca