# Choker 🤙

![image](https://github.com/user-attachments/assets/5acaa365-63a5-4d9e-a187-3a2f248c090e)


Choker is a sophisticated tool for ESP32 ULPs that enables seamless packet (via lwip, not the boring 802.11 frames) capturing and analysis by allowing the device to function in AP+STA mode with NAPT enabled, forwarding packets over serial for real-time monitoring in Wireshark. 
Still not getting the idea? Read on.

# Discover

![Choker1](https://github.com/user-attachments/assets/cafbced6-376c-483d-9020-ed7a703d05e8)


Curious about what those inexpensive WiFi 'smart home' devices are doing when plugged in? Want to know what your new router is transmitting upon activation? Or maybe you’re concerned about the suspicious activities of your smart washing machine? Spin up the Choker.

# Detailed Description

![image](https://github.com/user-attachments/assets/6a772f13-aa86-47a2-9e61-f8cc12808538)


Choker streamlines the network analysis process. How does it do that?

Instead of installing numerous tools and wrestling with command-line interfaces, simply power on your dedicated ESP32 and let it handle the heavy lifting.

Choker sets up an Access Point (AP) alongside Station (STA) mode (effectively extending your WiFi network yikes!) and, with NAPT enabled, forwards all packets to your Wireshark—this is where it gets particularly interesting. You can connect various devices like strange routers, IoT gadgets, and smart appliances to Choker's AP, allowing you to quickly observe what information they are transmitting and what data they might be siphoning off. 
By leveraging Network Address Port Translation (NAPT), Choker captures both incoming and outgoing packets and sends them over a serial connection to your connected computer, providing a comprehensive view of network activity.

## Have PHun!
