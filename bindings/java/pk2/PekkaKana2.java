package pk2;

public class PekkaKana2{
    public static void main(String args[]){
        System.out.println("Hello Java!");

        init("/Users/saturninufolud/c++/pk2_greta/bin/pk2.so",
        "/Users/saturninufolud/c++/pk2_greta/res");

        testLevel("Debug Island/tile_debug.map", false);
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
        setAssetsPath(assetsPath);
    }
    private static native void setAssetsPath(String path);
    public static native boolean testLevel(String name, boolean dev_mode);
}