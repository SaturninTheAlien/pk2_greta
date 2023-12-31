package pk2;

import java.nio.file.Path;
import java.nio.file.Paths;

import pk2.sprite.Prototype;
import pk2.sprite.PrototypesHandler;

public class PekkaKana2{
    public static void main(String args[]){
        System.out.println("Hello Java!");

        init();

        //testLevel("Debug Island/debug.map", true);

        PrototypesHandler handler = new PrototypesHandler(false, false);
        Prototype sprite = handler.loadPrototype("bat.spr2"); //handler.loadPrototype("episodes/Debug Island/evilkey.spr");

        System.out.println("Filename: "+sprite.getFilename());
        System.out.println("Name: "+sprite.getName());
        System.out.println("Type: "+sprite.getType());

        System.out.println("Width: "+sprite.getWidth());
        System.out.println("Height: "+sprite.getHeight());

        System.out.println("Texture: "+sprite.getTextureName());
        System.out.println("Color: "+sprite.getColor());
        
        System.out.println("Frame x: "+sprite.getFrameX());
        System.out.println("Frame y: "+sprite.getFrameY());
        System.out.println("Frame height: "+sprite.getFrameHeight());
        System.out.println("Frame width: "+sprite.getFrameWidth());

        quit();
    }

    public static void init(){
        Path rootPath = Paths.get("").toAbsolutePath().getParent().getParent();
        Path libPath = Paths.get(rootPath.toString(),"bin", "pk2_greta");
        Path resPath = Paths.get(rootPath.toString(), "res");
        init(libPath.toString(), resPath.toString());
    }

    public static void init(String dll_path, String assetsPath){
        
        String dllPathLowerCase = dll_path.toLowerCase();
        if(!dllPathLowerCase.endsWith(".so") && !dllPathLowerCase.endsWith(".dll")){
            
            String osname = System.getProperty("os.name").toLowerCase();
            System.out.println(osname);
            if(osname.contains("win")){
                dll_path+=".dll";
            }
            else{
                dll_path+=".so";
            }
        }
        System.load(dll_path);
        mInit(assetsPath);
    }

    private static native void mInit(String assetsPath);
        
    public static native void quit();
    public static native boolean testLevel(String name, boolean dev_mode);
}