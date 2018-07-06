/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sdljava.x.swig;

public class Mix_Chunk {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected Mix_Chunk(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Mix_Chunk obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      SWIG_SDLMixerJNI.delete_Mix_Chunk(swigCPtr);
    }
    swigCPtr = 0;
  }

  public void setAllocated(int allocated) {
    SWIG_SDLMixerJNI.set_Mix_Chunk_allocated(swigCPtr, allocated);
  }

  public int getAllocated() {
    return SWIG_SDLMixerJNI.get_Mix_Chunk_allocated(swigCPtr);
  }

  public void setAlen(long alen) {
    SWIG_SDLMixerJNI.set_Mix_Chunk_alen(swigCPtr, alen);
  }

  public long getAlen() {
    return SWIG_SDLMixerJNI.get_Mix_Chunk_alen(swigCPtr);
  }

  public void setVolume(short volume) {
    SWIG_SDLMixerJNI.set_Mix_Chunk_volume(swigCPtr, volume);
  }

  public short getVolume() {
    return SWIG_SDLMixerJNI.get_Mix_Chunk_volume(swigCPtr);
  }

  public Mix_Chunk() {
    this(SWIG_SDLMixerJNI.new_Mix_Chunk(), true);
  }

}