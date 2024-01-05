package pk2.sprite;

public class PrototypesHandler{

    public PrototypesHandler(boolean shouldLoadDependencies, boolean jsonPriority){
        mCreate(shouldLoadDependencies, jsonPriority);
    }

    public Prototype loadPrototype(String path){
        int sprite_id = this.mLoadSprite(path);
        if(sprite_id!=-1)return new Prototype(sprite_id);
        return null;
    }

    public int getID(){
        return this.id;
    }

    private native void mCreate(boolean shouldLoadDependencies, boolean jsonPriority);
    private native int mLoadSprite(String path);


    public native void setSearchingDir(String directory);
    public native void clear();
    
    private int id = -1;
};