package pk2;

public class Prototype{    
    protected Prototype(int sprite_id){
        this.sprite_id = sprite_id;
    }

    private int sprite_id = -1;

    public native int getType();
    public native String getFilename();
    public native String getName();

    public native int getWidth();
    public native int getHeight();

    public native String getTextureName();
    public native int getColor();
    public native int getFrameX();
    public native int getFrameY();
    public native int getFrameWidth();
    public native int getFrameHeight();
}