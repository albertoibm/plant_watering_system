class Pot {
  public:
    int moistureSensorPin;
    int relayPin;
    int dryThreshold;
    Pot *next;

    Pot(int moistureSensorPin, int relayPin, int dryThreshold) {
      this->moistureSensorPin = moistureSensorPin;
      this->relayPin = relayPin;
      this->dryThreshold = dryThreshold;
      this->next = nullptr;
    }

    void checkAndWater() {
      int moistureValue = analogRead(moistureSensorPin);
      Serial.print("Moisture Level (");
      Serial.print(moistureSensorPin);
      Serial.print("): ");
      Serial.println(moistureValue);

      if (moistureValue > dryThreshold) {
        digitalWrite(relayPin, LOW); // Turn on the pump
        delay(1000); // Run the pump for 1 second
        digitalWrite(relayPin, HIGH); // Turn off the pump
      }
    }
};

Pot *head = nullptr;

void addPot(int moistureSensorPin, int relayPin, int dryThreshold) {
  Pot *newPot = new Pot(moistureSensorPin, relayPin, dryThreshold);
  newPot->next = head;
  head = newPot;
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Turn off the pump at the beginning
}

void setup() {
  Serial.begin(9600);
  // Add initial pots here if needed, e.g.:
  // addPot(A0, 2, 600);
}

void loop() {
  Pot *currentPot = head;
  while (currentPot != nullptr) {
    currentPot->checkAndWater();
    currentPot = currentPot->next;
  }

  delay(30000); // Check the soil moisture every 30 seconds

  // Listen for serial commands to add pots on the fly
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    // Add pot with specified moisture sensor pin, relay pin, and dry threshold
    // Command format: "add,analogPin,digitalPin,dryThreshold"
    if (command.startsWith("add")) {
      int analogPin = command.substring(4, command.indexOf(',', 4)).toInt();
      int digitalPin = command.substring(command.indexOf(',', 4) + 1, command.lastIndexOf(',')).toInt();
      int dryThreshold = command.substring(command.lastIndexOf(',') + 1).toInt();
      addPot(analogPin, digitalPin, dryThreshold);
      Serial.print("Added pot with sensor pin: ");
      Serial.print(analogPin);
      Serial.print(", relay pin: ");
      Serial.print(digitalPin);
      Serial.print(", dry threshold: ");
      Serial.println(dryThreshold);
    }
  }
}
