package com.fennex.modules;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.nio.channels.FileLock;
import java.util.HashMap;

/**
 * Created by Fradow on 10/06/15.
 */
public class FileUtility implements ActivityResultResponder {
    private static final int FILE_PICK = 50;
    private static final String TAG = "FileUtility";
    HashMap<String, FileLock> currentLocks;
    HashMap<String, RandomAccessFile> currentFiles;
    private static volatile FileUtility instance = null;
    private static boolean isPending = false;
    private FileUtility() {
        currentLocks = new HashMap<String, FileLock>();
        currentFiles = new HashMap<String, RandomAccessFile>();
    }

    public static FileUtility getInstance()
    {
        if (instance == null)
        {
            synchronized (FileUtility .class)
            {
                if (instance == null)
                {
                    instance = new FileUtility();
                    if(NativeUtility.getMainActivity() != null) NativeUtility.getMainActivity().addResponder(instance);
                }
            }
        }
        return instance;
    }

    public void destroy()
    {
        if(isPending)
        {
            Toast.makeText(NativeUtility.getMainActivity(), TOO_MUCH_APP, Toast.LENGTH_LONG).show();
            isPending = false;
        }
        instance = null;
    }

    public static boolean lockFile(String filename) {
        boolean result = true;
        try {
            //Create parent directories recursively
            if(filename.lastIndexOf("/") != -1) {
                File directory = new File(filename.substring(0, filename.lastIndexOf("/") + 1));
                directory.mkdirs();
            }
            File fileBase = new File(filename);
            RandomAccessFile file = new RandomAccessFile(fileBase, "rwd");
            FileLock lock = file.getChannel().lock();
            getInstance().currentLocks.put(filename, lock);
            getInstance().currentFiles.put(filename, file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            result = false;
        } catch (IOException e) {
            e.printStackTrace();
            result = false;
        }
        return result;
    }

    public static String getLockedFileContents(String filename)
    {
        RandomAccessFile file = getInstance().currentFiles.get(filename);
        if(file == null) return null;
        String content = null;
        try {
            file.seek(0);
            byte[] data = new byte[(int) file.length()];
            file.readFully(data);
            content = new String(data, "UTF-8");
        } catch (IOException e) {
            e.printStackTrace();
        }
        return content;
    }

    public static boolean writeLockedFile(String filename, String content)
    {
        RandomAccessFile file = getInstance().currentFiles.get(filename);
        if(file == null) return false;
        boolean result = true;
        try {
            //Rewind and erase data
            file.seek(0);
            file.setLength(0);
            //Overwrite with new content
            file.write(content.getBytes("UTF-8"));
        } catch (IOException e) {
            result = false;
            e.printStackTrace();
        }
        return result;
    }

    public static void unlockFile(String filename) {
        FileLock lock = getInstance().currentLocks.get(filename);
        RandomAccessFile file = getInstance().currentFiles.get(filename);
        if(lock != null && file != null) {
            if(!lock.isValid()) Log.i("FileUtility", "Lock is invalid for file : "+ filename);
            try {
                lock.release();
                file.close();
                getInstance().currentLocks.remove(filename);
                getInstance().currentFiles.remove(filename);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void deleteRecursive(File fileOrDirectory) {
        if (fileOrDirectory.isDirectory())
            for (File child : fileOrDirectory.listFiles())
                deleteRecursive(child);
        
        fileOrDirectory.delete();
    }

    public static String[] getFilesInFolder(String folderPath)
    {
        File dir = new File(folderPath);
        String[] files = new String[]{};
        if(dir.exists() && dir.isDirectory())
        {
            files = dir.list();
        }
        return files;
    }

    public static void deleteFile(String filename)
    {
        unlockFile(filename);
        File file = new File(filename);
        if(!file.delete())
        {
            Log.i("FileUtility", "Could not delete file : " + filename);
        }
    }

    public static boolean moveFileToLocalDirectory(String path)
    {
        String filename = path.substring(path.lastIndexOf("/"));
        File destinationFile = new File(NativeUtility.getLocalPath() + java.io.File.separator + filename);

        if(!destinationFile.exists())
        {
            InputStream in = null;
            OutputStream out = null;
            try
            {
                in = new FileInputStream(path);
                out = new FileOutputStream(NativeUtility.getLocalPath() + java.io.File.separator + filename);
                byte[] buffer = new byte[1024];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
                in.close();
                in = null;

                // write the output file
                out.flush();
                out.close();
                out = null;

                // delete the original file
                new File(path).delete();
            }
            catch(Exception e)
            {
                Log.i("FileUtility", "Could not move file : " + filename);
                e.printStackTrace();
                return false;
            }
        }
        else
        {
            Log.i("FileUtility", "didn't copied" + filename + ", already exist");
            try
            {
                // delete the original file
                new File(path).delete();
            }
            catch(Exception e)
            {
                Log.i("FileUtility", "Couldn't delete file :"+ path);
                e.printStackTrace();
            }
        }
        return true;
    }

    public static boolean pickFile()
    {
        FileUtility.getInstance(); //ensure the instance is created
        boolean error = false;
        try
        {
            Intent intent;
            intent = new Intent(Intent.ACTION_GET_CONTENT, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            intent.putExtra(Intent.EXTRA_LOCAL_ONLY, true);
            intent.setType("file/*");
            isPending = true;
            NativeUtility.getMainActivity().startActivityForResult(intent, FILE_PICK);
        }
        catch(ActivityNotFoundException e)
        {
            Log.d(TAG, "intent for image pick from File library not found : " + e.getMessage());
            error = true;
        }
        return error;
    }

    @Override
    public boolean onActivityResult(int requestCode, int resultCode, Intent data) {
        isPending = false;
        if (requestCode == FILE_PICK && NativeUtility.getMainActivity() != null) {
            Log.d(TAG, "intent data: " + data.getDataString());
            final Uri fileUri = data.getData();
            NativeUtility.getMainActivity().runOnGLThread(new Runnable()
            {
                public void run()
                {
                    notifyFilePicked(fileUri.getPath());
                }
            });
            return true;
        }
        return false;
    }

    public native static void notifyFilePicked(String path);
}
