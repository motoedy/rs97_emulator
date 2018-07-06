/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sdljava.x.swig;

public class SDL_MouseButtonEvent {
    public long swigCPtr;
  public boolean swigCMemOwn;

  protected SDL_MouseButtonEvent(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(SDL_MouseButtonEvent obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      SWIG_SDLEventJNI.delete_SDL_MouseButtonEvent(swigCPtr);
    }
    swigCPtr = 0;
  }

  public void setType(short type) {
    SWIG_SDLEventJNI.set_SDL_MouseButtonEvent_type(swigCPtr, type);
  }

  public short getType() {
    return SWIG_SDLEventJNI.get_SDL_MouseButtonEvent_type(swigCPtr);
  }

  public void setWhich(short which) {
    SWIG_SDLEventJNI.set_SDL_MouseButtonEvent_which(swigCPtr, which);
  }

  public short getWhich() {
    return SWIG_SDLEventJNI.get_SDL_MouseButtonEvent_which(swigCPtr);
  }

  public void setButton(short button) {
    SWIG_SDLEventJNI.set_SDL_MouseButtonEvent_button(swigCPtr, button);
  }

  public short getButton() {
    return SWIG_SDLEventJNI.get_SDL_MouseButtonEvent_button(swigCPtr);
  }

  public void setState(short state) {
    SWIG_SDLEventJNI.set_SDL_MouseButtonEvent_state(swigCPtr, state);
  }

  public short getState() {
    return SWIG_SDLEventJNI.get_SDL_MouseButtonEvent_state(swigCPtr);
  }

  public void setX(int x) {
    SWIG_SDLEventJNI.set_SDL_MouseButtonEvent_x(swigCPtr, x);
  }

  public int getX() {
    return SWIG_SDLEventJNI.get_SDL_MouseButtonEvent_x(swigCPtr);
  }

  public void setY(int y) {
    SWIG_SDLEventJNI.set_SDL_MouseButtonEvent_y(swigCPtr, y);
  }

  public int getY() {
    return SWIG_SDLEventJNI.get_SDL_MouseButtonEvent_y(swigCPtr);
  }

  public SDL_MouseButtonEvent() {
    this(SWIG_SDLEventJNI.new_SDL_MouseButtonEvent(), true);
  }

}