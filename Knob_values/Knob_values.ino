void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.print("A10: ");
  Serial.print(analogRead(A10));
  Serial.print(" | A11: ");
  Serial.print(analogRead(A11));
  Serial.print(" | A12: ");
  Serial.print(analogRead(A12));
  Serial.print(" | A13: ");
  Serial.print(analogRead(A13));
  Serial.print(" | A14: ");
  Serial.print(analogRead(A14));
  Serial.print(" | A15: ");
  Serial.print(analogRead(A15));
  Serial.print(" | A16: ");
  Serial.print(analogRead(A16));
  Serial.print(" | A17: ");
  Serial.print(analogRead(A17));
  
  Serial.println("");
}
