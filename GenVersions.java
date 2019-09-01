import java.io.InputStream;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Set;
import java.util.HashSet;
import java.util.Stack;

public class GenVersions
{
  static Stack<Condition> stack = new Stack<Condition>();

  static class Condition
  {
    String def;
    boolean set;

    public Condition(String def, boolean set)
    {
      this.def = def;
      this.set = set;
    }
  }

  static boolean canPrint(Set<String> defines)
  {
    for (Condition c : stack) {
      if ((c.def != null) && (c.def.equals("USERLAND")))
        continue;
      if (c.def == null) {
        if (!c.set) // #if 0
          return false;
      }
      else if (c.set && !defines.contains(c.def)) // #ifdef/else
        return false;
      else if (!c.set && defines.contains(c.def)) // #ifndef/else
        return false;
    }
    return true;
  }

  static boolean processStream(InputStream in, OutputStream out,
                            Set<String> defines)
    throws IOException
  {
    InputStreamReader iread = new InputStreamReader(in);
    BufferedReader bread = new BufferedReader(iread);
    PrintWriter pw = new PrintWriter(out);
    String line;
    boolean hasContent = false;
    while (null != (line = bread.readLine())) {
      String trimmed = line.trim();

      if (trimmed.startsWith("#if ")) {
        String def = trimmed.substring(4);
        if (def.equals("0"))
          stack.push(new Condition(null,false));
        else
          stack.push(new Condition(null,true));
      }
      else if (trimmed.startsWith("#ifdef ")) {
        String def = trimmed.substring(7);
        if ((def != null) && !def.startsWith("USE_") && canPrint(defines))
          pw.println(line);
        stack.push(new Condition(def,true));
      }
      else if (trimmed.startsWith("#ifndef ")) {
        String def = trimmed.substring(8);
        if ((def != null) && !def.startsWith("USE_") && canPrint(defines))
          pw.println(line);
        stack.push(new Condition(def,false));
      }
      else if (trimmed.startsWith("#else")) {
        Condition cond = stack.pop();
        stack.push(new Condition(cond.def,!cond.set));
        if ((cond.def != null) && !cond.def.startsWith("USE_") &&
            canPrint(defines))
          pw.println(line);
      }
      else if (trimmed.startsWith("#endif")) {
        Condition cond = stack.pop();
        if ((cond.def != null) && !cond.def.startsWith("USE_") &&
            canPrint(defines))
          pw.println(line);
      }
      else if (trimmed.startsWith("#define USE_")) {
        // skip
      }
      else {
        if (canPrint(defines)) {
          pw.println(line);
          if ((trimmed.length() > 0) &&
              !trimmed.startsWith("#include"))
            hasContent = true;
        }
      }
    }
    pw.flush();
    return hasContent;
  }

//   static String[] versionDefines = {};
  static String[] versionDefines =
  {"USE_LIBC","USE_PROCESSES","USE_PAGING","USE_SYSCALLS","USE_MALLOC",
   "USE_FILEDESC","USE_UNIXPROC","USE_ANSWERS"};


  public static void main(String[] args)
    throws IOException
  {
    if (args.length < 2) {
      System.err.println("Usage: GenVersions <srcdir> <verdir>");
      System.exit(-1);
    }

    File sourceDir = new File(args[0]);
    File verDir = new File(args[1]);

    File[] verContents = verDir.listFiles();
    if (verContents == null) {
      System.err.println(verDir+": not a directory");
      System.exit(-1);
    }
//     if (verContents.length != 0) {
//       System.err.println(verDir+": directory not empty");
//       System.exit(-1);
//     }


    for (int version = 0; version <= versionDefines.length; version++) {
      System.out.println("version"+version);
      Set<String> defSet = new HashSet<String>();

      for (int v = 1; v <= version; v++)
        defSet.add(versionDefines[v-1]);

      File thisVerDir = new File(verDir+"/version"+(version+1));
      thisVerDir.mkdir();

      for (File sourceFile : sourceDir.listFiles()) {
        String filename = sourceFile.getName();
        if (filename.endsWith(".c") ||
            filename.endsWith(".h") ||
            filename.endsWith(".s") ||
            filename.endsWith(".ld") ||
            filename.endsWith("mkbootimage.sh") ||
            filename.equals("Makefile")) {

          System.out.println("  "+filename);
          assert(stack.size() == 0);

          File outFile = new File(thisVerDir.getPath()+"/"+filename);

          InputStream in = new FileInputStream(sourceFile);
          OutputStream out = new FileOutputStream(outFile);
          boolean hasContent = processStream(in,out,defSet);
          out.close();
          in.close();

          if (!hasContent)
            outFile.delete();
        }
      }
    }
  }
}
