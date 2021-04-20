# Installation

This script will install all additional needed packages for [original Raspbian](https://downloads.raspberrypi.org/raspios_armhf/images/raspios_armhf-2021-03-25/2021-03-04-raspios-buster-armhf.zip).

---------------------------

At the end of the script, there will be some user input needed for MySQL installation and VPN installation.

## MySQL installation
You can follow [online guide](https://pimylifeup.com/raspberry-pi-mysql/) or you follow the steps here.

---------------------------

This will run the secure installation where you should select Y(Yes) or N(No) for all the prompts and set the password.

``sudo mysql_secure_installation``

To access the database run the following command and enter your password.

``sudo mysql -u root -p``

To setup the database for the BP you should run the following commands.

To create database

``CREATE DATABASE SmartHomedb;``

To create user, you can use your own username and password.

``CREATE USER 'uiDesktopApp' IDENTIFIED BY 'passForUiDesktopApp';``

Handle privileges.

``GRANT ALL PRIVILEGES ON smartHomedb.* TO 'uiDesktopApp';``
``FLUSH PRIVILEGES;``

Create the messages table for the Smart Home App.

``CREATE TABLE messages(messageId INT NOT NULL AUTO_INCREMENT, topic VARCHAR(500) NOT NULL, value VARCHAR(500), room VARCHAR(255), message_recieved TIMESTAMP , PRIMARY KEY (messageId) );``

## VPN installation
You can follow [online guide](https://pimylifeup.com/raspberry-pi-wireguard/) or you follow the steps here.

---------------------------

Start the installation script

``curl -L https://install.pivpn.io | bash``

The script will go through the installation process.

While choosing the VPN, choose Wireguard.

On Public IP or DNS you can choose either I recommend the DNS entry.

---------------------------

After the installation is done you should generate the VPN profile. 
You should create separate profile for each device.

To create the profile you should run this command and select the profile name.

``sudo pivpn add``

The config file will be saved in: `/home/USERNAME/configs` and you can use it to setup wireguard client.

If you are using mobile device as a client you can use the generated QR code.

To generate QR code you can run following command and scan the generated QR code with the Wireguard app.

``pivpn -qr PROFILENAME``