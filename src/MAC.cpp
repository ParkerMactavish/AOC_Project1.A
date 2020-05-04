#include "MAC.h"

void MAC::multi(){
  this->poResult = this->piWeight.read() * this->piInput.read();
}