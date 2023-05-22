#include <math.h>
#define INTERVAL 30*1000
#define MAXPOTS 10
bool verbose = true;
int potCount = 0;
class Pot {
  public:
    int potId;
    int moistureSensorPin;
    int relayPin;
    int dryThreshold;
    int volume = 0; // Volume of the pot in milliliters
    int diameter = 0; // Diameter of the pot in centimeters
    int moistureValue = 0;
    int accWater = 0;
    unsigned long timeTag = 0;
    Pot *next;

    Pot(int moistureSensorPin, int relayPin, int dryThreshold, int diam) {
      this->potId = potCount++; // set potId and increase count
      this->moistureSensorPin = moistureSensorPin;
      this->relayPin = relayPin;
      this->dryThreshold = dryThreshold;
      setDiam(diam);
      this->next = nullptr;
    }

    void setDiam(int diam) {
      diameter = diam;
      // Assume height = 2/3 of diameter
      setVol(((double)diam * 0.66) * 3.1415926535 * (diam * diam / 4)); // V = h * pi * r ^ 2
    }
    void setVol(int vol) {
      volume = vol;
      if (diameter == 0) {
        diameter = pow(volume / 3.1415926535 * 4, 0.3333333);
      }
    }
    int calculateWater() {
      int water;
      if (volume == 0) {
        water = 50; // give 50ml, to not overwater the pot
      } else {
        water = volume * 0.01; // Give 1% of pot's volume. TODO include level of humidity
      }
      if (verbose) {
        Serial.print("Give ");
        Serial.print(water);
        Serial.println(" ml");
      }
      return water;
    }
    int calculateTime(int water) {
      double a = 0.0202436, b = -14.75791673;
      int tim = max(0, (int)(((double)water - b) / a));
      return tim;
    }
    void doWater(int water) {
      unsigned long timePassed = millis() - timeTag;
      if (timeTag == 0 || timePassed > (1000*3600*24)){ // A day has passed or it's the first time?
        timeTag = millis();  // Reset timetag
        accWater = 0; // and water given this day
      }
      
      if (accWater+water < (volume * 0.1)) {
        int tim = calculateTime(water);
        if (verbose){
          Serial.print("Watering for ");
          Serial.print(tim);
          Serial.println(" ms.");
        }
        digitalWrite(relayPin, HIGH);// Turn on the pump
        delay(tim);
        digitalWrite(relayPin, LOW);// Turn off the pump
        accWater += water; // Integrate water given
      } else {
        Serial.print("Cannot give more than 10% of the pot's volume ");
        Serial.print((int)((double)volume * 0.1));
        Serial.println(" ml in water in a single day");
      }
    }
    void readMoisture() {
      moistureValue = analogRead(moistureSensorPin);
    }
    void checkAndWater() {
      Serial.println("checkAndWater()");
      
      int water;
      readMoisture();
      Serial.print("Moisture Level (");
      Serial.print(moistureSensorPin);
      Serial.print("): ");
      Serial.println(moistureValue);

      if (moistureValue < dryThreshold) {
        if (verbose) Serial.println("Plant is dry");
        water = calculateWater();
        doWater(water);
      }
    }
};

Pot *head = nullptr;

void addPot(int moistureSensorPin, int relayPin, int dryThreshold, int diameter) {
  Pot *newPot = new Pot(moistureSensorPin, relayPin, dryThreshold, diameter);
  newPot->next = head;
  head = newPot;
  if (verbose){
    Serial.println("Added new pot:");
    Serial.print("Pot ID: ");
    Serial.println(newPot->potId);
  }
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Turn off the pump at the beginning
}
bool validateCommand(const String& command, int expectedArgs) {
    int numArgs = 0;
    int commaIndex = command.indexOf(',');
    int startIndex = (commaIndex != -1) ? commaIndex + 1 : 0;

    for (int i = startIndex; i < command.length(); i++) {
        if (command.charAt(i) == ',') {
            numArgs++;
        }
    }

    // Add 1 to account for the command itself
    numArgs++;
    // Add 1 to account for the first comma
    if (startIndex) numArgs++;

    if (numArgs != expectedArgs) {
      Serial.println("Wrong number of arguments");
      Serial.print("Expected: ");
      Serial.print(expectedArgs);
      Serial.print(", received: ");
      Serial.println(numArgs); 
      return false;
    }

    for (int i = startIndex; i < command.length(); i++) {
        if (command.charAt(i) == ',' || isDigit(command.charAt(i))) {
            continue;
        } else {
          Serial.print("Found wrong character in command: (");
          Serial.print(command.charAt(i));
          Serial.println(")"); 
          return false;
        }
    }

    // Check if the last character is a comma
    if (command.charAt(command.length() - 1) == ',') {
      Serial.println("Last char is a comma");
        return false;
    }

    return true;
} 
 
void processSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (verbose){
      Serial.print("Received command: ");
      Serial.println(command);
    }
    // Add pot with specified moisture sensor pin, relay pin, and dry threshold
    // Command format: "add,analogPin,digitalPin,dryThreshold,diameter"
    // ADD COMMAND
    if (command.startsWith("add") && validateCommand(command, 5)) {
      int analogPin = command.substring(4, command.indexOf(',', 4)).toInt();
      int digitalPin = command.substring(command.indexOf(',', 4) + 1, command.lastIndexOf(',')).toInt();
      int dryThreshold = command.substring(command.lastIndexOf(',', command.lastIndexOf(',') - 1) + 1, command.lastIndexOf(',')).toInt();
      int diameter = command.substring(command.lastIndexOf(',') + 1).toInt();
      addPot(analogPin, digitalPin, dryThreshold, diameter);
      Serial.print("Added pot with sensor pin: ");
      Serial.print(analogPin);
      Serial.print(", relay pin: ");
      Serial.print(digitalPin);
      Serial.print(", dry threshold: ");
      Serial.print(dryThreshold);
      Serial.print(", diameter: ");
      Serial.println(diameter);
    // READ COMMAND
    } else if (command.startsWith("read") && validateCommand(command, 2)) {
      // This command asks for the details of the pot with PotID = id
      int id = command.substring(5).toInt(); // Extract PotID from the command

      Pot *currentPot = head;
      while (currentPot != nullptr) {
        if (currentPot->potId == id) {
          // Pot found, send its details over Serial
          Serial.print("PotID: ");
          Serial.println(currentPot->potId);
          Serial.print("Moisture: ");
          Serial.println(currentPot->moistureValue);
          Serial.print("Moisture Sensor Pin: ");
          Serial.println(currentPot->moistureSensorPin);
          Serial.print("Water given [ml]: ");
          Serial.println(currentPot->accWater);
          Serial.print("Relay Pin: ");
          Serial.println(currentPot->relayPin);
          Serial.print("Dry Threshold: ");
          Serial.println(currentPot->dryThreshold);
          Serial.print("Volume: ");
          Serial.print(currentPot->volume);
          Serial.println(" ml");
          Serial.print("Diameter: ");
          Serial.print(currentPot->diameter);
          Serial.println(" cm");

          break; // Exit the loop after finding the pot
        }

        currentPot = currentPot->next;
      }

      // If pot is not found, send an error message
      if (currentPot == nullptr) {
        Serial.println("Pot not found.");
      }
    // MOIST COMMAND
    } else if (command.startsWith("moist") && validateCommand(command, 2)) {
      // This command asks for the moisture level of the pot with PotID = id
      int id = command.substring(5).toInt(); // Extract PotID from the command
  
      Pot *currentPot = head;
      while (currentPot != nullptr) {
        if (currentPot->potId == id) {
          currentPot->readMoisture();
          // Pot found, send its details over Serial
          Serial.print("PotID:");
          Serial.print(currentPot->potId);
          Serial.print(",Moisture:");
          Serial.println(currentPot->moistureValue);
  
          break; // Exit the loop after finding the pot
        }
  
        currentPot = currentPot->next;
      }
  
      // If pot is not found, send an error message
      if (currentPot == nullptr) {
        Serial.println("Pot not found.");
      }
      // DELETE COMMAND
    } else if (command.startsWith("del") && validateCommand(command, 2)) {
    // This command deletes a pot
      int id = command.substring(4).toInt(); // Extract PotID from the command
    
      Pot* currentPot = head;
      Pot* previousPot = nullptr;
    
      // Traverse the linked list to find the pot with the given potId
      while (currentPot != nullptr) {
        if (currentPot->potId == id) {
          // Pot with matching potId found, remove it from the list
          if (previousPot == nullptr) {
            // If the pot to be removed is the head, update the head pointer
            head = currentPot->next;
          } else {
            // If the pot to be removed is not the head, update the next pointer of the previous pot
            previousPot->next = currentPot->next;
          }
          delete currentPot; // Free the memory allocated for the pot
          if (verbose) {
            Serial.print("Pot with Pot ID ");
            Serial.print(id);
            Serial.println(" deleted.");
          }
          return;
        }
    
        // Move to the next pot
        previousPot = currentPot;
        currentPot = currentPot->next;
      }
    
      // Pot with the given potId not found
      if (verbose) {
        Serial.print("Pot with Pot ID ");
        Serial.print(id);
        Serial.println(" not found.");
      }
    // HEAD COMMAND
    } else if (command.startsWith("head") && validateCommand(command, 1)) {
      // This command asks for the pot Id of the head pot (first in the list) 
  
      Pot *currentPot = head;
      if (currentPot == nullptr) {
        Serial.println("Pot not found.");
      } else {
        Serial.print("PotID:");
        Serial.println(currentPot->potId);
      }
      // If pot is not found, send an error message
      // WATER COMMAND
    } else if (command.startsWith("water") && validateCommand(command, 3)) {
      Serial.println("Watering...");
      int potId = command.substring(6, command.indexOf(',', 6)).toInt();
      int volume = command.substring(command.indexOf(',', 6) + 1).toInt();
      Pot *currentPot = head;
      while (currentPot != nullptr) {
          if (currentPot->potId == potId) {
              currentPot->doWater(volume);
              break;
          }
          currentPot = currentPot->next;
      }
    } else if (command.startsWith("help")) {
      // Print help instructions
      Serial.println("Available commands:");
      Serial.println("- add,analogPin,digitalPin,dryThreshold,diameter: Add a new pot with the specified parameters");
      Serial.println("- read,PotID: Read the details of the pot with the specified PotID");
      Serial.println("- moist,PotID: Read the details moisture level of the pot with the specified PotID");
      Serial.println("- water,PotID,volume: Water the pot with the specified PotID with 'volume' ml.");
      Serial.println("- help: Show this help message");
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Plant watering system.");
  Serial.println("Starting program.");
  // Add initial pots here if needed, e.g.:
  // addPot(0, 3, 700, 29);
}

void loop() {
  Pot *currentPot = head;
  int readPots = 0;
  Serial.println("Checking pots..");
  while (currentPot != nullptr && readPots < MAXPOTS) {
    currentPot->checkAndWater();
    currentPot = currentPot->next;
    readPots++;
  }
  unsigned long cycleTime = millis();
  while (millis() - cycleTime < INTERVAL){ // Check the soil moisture every INTERVAL seconds
    delay(300); // wait 300ms  
    processSerialCommands();
    //Serial.print(".");
  }
  //Serial.println("End loop()");
}
