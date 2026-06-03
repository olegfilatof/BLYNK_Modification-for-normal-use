
 Proof of implementation for two concepts:
 1. Dynamic connection to Blynk by entering BLYNK_AUTH_TOKEN in the browser (no additional code inside the Blynk library is now required!!!)
 2. A universal bridge for all Blynk connection types. I created
 Dynamic connection based on a flag from Blynk (or from a sensor, button, etc.) for Wi-Fi and modem connections.

It works flawlessly. For example, I implemented it: if Wi-Fi stops working, the modem reconnects and disables the controller's Wi-Fi. Data is sent to MQTT, Node Red, or the Blynk server without rebooting. Now even engineers can use Blynk, as true connection redundancy has been introduced, overcoming the ridiculous limitation of Blynk's creators. You can see all in code.

 ( I also developed asynchronous connection code for the modem and Wi-Fi, which allows the system to execute code (sensors, buttons) even while the modem and Wi-Fi are connecting. And the ability to transmit GPS coordinates through the Blynk app and view them on Google Maps in the free version. I don't intend to offer it for free yet. But rest assured, this actions are realy possible. }
 
