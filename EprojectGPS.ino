#include <SoftwareSerial.h> // Allow serial communication on other digital pins
#include <String.h>
//#include <TinyGPS++.h>

SoftwareSerial myown(7, 8); // RX, TX
String incomingString = "", information;
String tempstr, number = "670865568"; // Replace with the recipient phone number
String gpsData1;
bool gpsDataAvailable = false;

void sendsms(String sms);
bool validateGPRMC(String gprmc);  // Function to validate the $GPRMC sentence

void setup() {
  Serial.begin(9600);
  myown.begin(9600);

  myown.println("AT"); // Check communication between the module and the computer
  delay(2000);

  myown.println("AT+CMGF=1"); // Set the GSM module to SMS mode
  delay(2000);

  myown.println("AT+CNMI=2,1,0,1,0"); // New message indications to TE
  delay(2000);

  myown.println("AT+CSMP=49,167,0,0"); // Set text mode parameters
  delay(2000);

  myown.println("AT+CMGD=1,4"); // Delete messages
  delay(2000);

  Serial.println();
  myown.println("AT+CMGDA=\"DEL ALL\"");
  delay(2000);
}

void loop() {
  // GPS data reading
  if (Serial.available()) {
    gpsData1 = Serial.readStringUntil('\n');
     Serial.println("Raw GPS data: " + gpsData1);
    if (gpsData1.startsWith("$GPRMC")) {
      Serial.println("Recieved $GPRMC Sentence: " + gpsData1);
      // Validate the $GPRMC sentence
      if (validateGPRMC(gpsData1)) {
        // Split GPS data into individual values
        String values[13];
        int index = 0;
        int startIndex = 0;
        int endIndex = gpsData1.indexOf(',');

        while (endIndex != -1) {
          values[index++] = gpsData1.substring(startIndex, endIndex);
          startIndex = endIndex + 1;
          endIndex = gpsData1.indexOf(',', startIndex);
        }
        values[index] = gpsData1.substring(startIndex);

        // Extract relevant data
        String longitud = values[5];
        delay(1000);
        String longitude_dir = values[6];
        delay(1000);
        String latitud = values[3];
        delay(1000);
        String latitude_dir = values[4];
        delay(1000);
        String tim = values[1].substring(0, 2) + ":" + values[1].substring(2, 4) + ":" + values[1].substring(4, 6);
        delay(1000);
        String date = values[9].substring(0, 2) + "/" + values[9].substring(2, 4) + "/" + values[9].substring(4, 6);
        delay(1000);

        // Convert latitude and longitude to decimal format
        float lat = convertToDecimal(latitud, latitude_dir);
        float lon = convertToDecimal(longitud, longitude_dir);

        // Generate Google Maps link
        String googleMapsLink = "https://www.google.com/maps?q=" + String(lat, 6) + "," + String(lon, 6);
        delay(2000);

        // Construct SMS information
        information = "Longitude: " + longitud + "\nLongitude Direction: " + longitude_dir + 
                      "\nLatitude: " + latitud + "\nLatitude Direction: " + latitude_dir + 
                      "\nDate: " + date + "\nTime: " + tim + 
                      "\nGoogle Maps Link: " + googleMapsLink;
                       delay(1000);

        gpsDataAvailable = true;  // GPS data is now available
        Serial.println("GPS Data Captured:");
        Serial.println(information);
     } else {
        Serial.println("Incomplete or corrupt $GPRMC sentence.");
      }
    }
  }

  // GSM SMS reading and sending
  if (myown.available() > 0) {
    incomingString = myown.readString();
    Serial.println(incomingString);

    if (incomingString.indexOf("+CMTI") > -1) { // SMS available
      tempstr = incomingString.substring(incomingString.indexOf(',') + 1);
      myown.print("AT+CMGR="); // Read message
      myown.println(tempstr);
    } else if (incomingString.length() > 20 && incomingString.indexOf("+CMGR: ") > -1) { 
      // Incoming SMS - respond with GPS info
       delay(2000);
      if (gpsDataAvailable) {
        sendsms(information);  // Send the GPS info as SMS
      } else {
        sendsms("GPS data not found yet. Please try again later.");  // Ask user to try again
      }
    }
  } else {
    Serial.println("No command received");
    // Serial.println(gpsData1);
  }
}

void sendsms(String sms) {
  myown.println("AT+CMGF=1"); // Set GSM module to Text Mode
  delay(1000);
  myown.println("AT+CMGS=\"" + number + "\"\r"); // Send SMS to the number
  delay(1000);
  myown.println(sms); // SMS content
  delay(500);
  myown.println((char)26); // ASCII code for CTRL+Z to send SMS
  delay(500);
  myown.print("AT+CMGDA=\"");
  myown.println("DEL ALL\""); // Delete all messages
  delay(500);
}

// Function to validate the $GPRMC sentence
bool validateGPRMC(String gprmc) {
  int numChars = 0;
  for (int i = 0; i < gprmc.length(); i++) {
    if (gprmc.charAt(i) != ',') {
      numChars++;
    }
  }
  // A valid $GPRMC sentence typically has 12 commas
  return (numChars > 50);
}

// Function to convert latitude or longitude to decimal degrees
float convertToDecimal(String coordinate, String direction) {
  float raw = coordinate.toFloat();
  int degrees = int(raw / 100); // Extract degrees part
  float minutes = raw - (degrees * 100); // Extract minutes part
  float decimal = degrees + (minutes / 60); // Convert to decimal degrees

  // Adjust for hemisphere (N/S or E/W)
  if (direction == "S" || direction == "W") {
    decimal *= -1;
  }

  return decimal;
}
