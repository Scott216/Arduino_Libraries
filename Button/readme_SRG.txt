1/26/13 - added function that would reset onPressAsToggle() function

All it's doing is resetting _pToggleFlag

void Button::onPressAsToggleRst(void){
  _pToggleFlag = false;
}
