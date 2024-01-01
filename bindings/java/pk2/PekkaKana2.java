package pk2;

import java.io.File;
import java.io.FilenameFilter;

import java.nio.file.Path;
import java.nio.file.Paths;

import pk2.sprite.Prototype;
import pk2.sprite.PrototypesHandler;

public class PekkaKana2{

    private static boolean isWindows(){
        String osname = System.getProperty("os.name").toLowerCase();
        return osname.contains("win");
    }

    public static void main(String args[]){
        System.out.println("Hello Java!");

        init();

        //testLevel("Debug Island/tile_debug.map", true);

        PrototypesHandler handler = new PrototypesHandler(false, false);
        handler.clear();
        handler.setSearchingDir(".");
        
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

        String s1 = isWindows()?"res":"bin";
        
        Path libPath = Paths.get(rootPath.toString(), s1, "pk2_greta");
        Path resPath = Paths.get(rootPath.toString(), "res");
        init(libPath.toString(), resPath.toString());
    }

    private static void loadDependentDLLs(String dll_path){
        File dir = new File(dll_path).getParentFile();

        /**
         * Load dependent DLLs
         */

        FilenameFilter dllFilter = new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                String s = name.toLowerCase();

                return s.endsWith(".dll") && !name.startsWith("pk2");
            }
        };


        File[] files = dir.listFiles(dllFilter);
        if(files!=null){
            for (File dllFile : files){
                String s = dllFile.getAbsolutePath();
                System.out.println("Loading: "+s);
                System.load(s);
            }
        }   
    }

    public static void init(String dll_path, String assetsPath){
        
        String dllPathLowerCase = dll_path.toLowerCase();
        if(!dllPathLowerCase.endsWith(".so") && !dllPathLowerCase.endsWith(".dll")){
            if(isWindows()){
                dll_path+=".dll";                            
            }
            else{
                dll_path+=".so";
            }
        }
        if(isWindows()){
            loadDependentDLLs(dll_path);
        }

        System.load(dll_path);
        mInit(assetsPath);
    }

    private static native void mInit(String assetsPath);
        
    public static native void quit();
    public static native boolean testLevel(String name, boolean dev_mode);
}