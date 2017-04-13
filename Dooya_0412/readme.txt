1. Introduction
===============
	Generally a certain customer may have multi product lines, such as air conditioner, dehumidifier, air fresher, water purifier, etc. All these kinds of product can use a customized common serial protocol to enhance new features. This is the way we use to make them smart. We design a mini MCU + Wi-Fi module board. It can talk to the product who wants to be smart. It also talk to Ayla cloud to get remote connectivation. We call this board as convertion board.

	Currently we designed the convertion board using STM32F103 and one of Ayla embedded Wi-Fi module. This package contains all necessary software for air conditioner and dehumidifier. Customer can design their own project based on this reference code.

2. File organization
====================
	The source code is organized as following structure.
root
+--arch
|   |---img_pkg
|   +---STM32F10x_StdPeriph_Driver
+---example
|   |---common
|   |---dehumidifier
|   |---file_prop
|   |---img_mgmt
|   |---smart_ac
|   +---wifi_mgmt
+---lib
|   +---ayla
|       |---config
|       |---file_prop
|       |---host_ota
|       |---include
|       |---sched
|       |---spi
|       +---uart
+---proj
    +---stm32f1
        +---keil

	The directory root\arch contains hardware dependency code for driver. If the customer wants to change the hardware design, he may need to modify these code.

	The directory root\lib contains the driver to Ayla embedded Wi-Fi module. They should not be modified by users.

	The directory root\proj\stm32f1\keil contains Keil projects for STM32F103 which is the host MCU interfacing to Wi-Fi module. There are two parts of projects. The first part is loader.uvproj which is common project for all applications. The other part are sac_rls.uvproj and deh_rls.uvproj. They are application projects for air conditioner and dehumidifier. A full MCU image is combinded of loader and one of the applications. It means the user should download the loader project firstly. Then download the application project secondly.

	All application code are stored in root\example. Here you can find smart_ac and dehumidifier. According to the mechanism maintained above, we seperated the code in two parts. We grouped the common code for all applications in root\example\common. And put the private code belongs to the application in dedicated directory. Here are root\example\dehumidifier and root\example\smart_ac. In each directory, the code are put in three files according to their functions. They are:

	xxx_protocol.c/h	Serial protocol belongs to this application.
	xxx_property.c/h	Property table and set/get functions of each property.
	xxx_control.c/h		Application initialization, main work flow and misc functions.

3. Key items & Tips
===================
3.1 The main function entry is in app_main.c. In initial stage, app_init() is called for common initialization for serial communication and message queue. Then custom_init() is called to do the application initializations.

3.2 The main loop is in main(). All polling functions are called sequently, including the common polling function app_poll(). The application polling function custom_poll() is called in app_poll(). Please pay attension that you should not make the MCU blocked in your code. Otherwise, the system may ruan abnormally.

3.3 There are some common properties which are necessary for each application. They are defined in app_property.h, along with their set/get functions and local variables. These contents should be used in each application project.

3.4 Since most of smart products can be controlled simultaneously by remote APP and local IR controller. We use a message queue to buffer the requests from cloud(remote) to avoid the control conflict between remote and local. The operation from IR controller is directly accepted by customer control board and the result is synced to the cloud soon. The operation from cloud is put in the queue first and will be served when there is no local control.
