package simuladordebanco;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.file.Files;
import java.nio.file.Paths;

public class DataFile {

    //65.536 datablocks
    long size;
    String path = "DataFile.bin";

    public DataFile() throws IOException {
        size = 268435456;//268.435.456
        //createBinary(); //Just need to run one time to create the file
    }
    
    /*
        create the binary file.
     */
    public void createBinary() throws IOException {
        byte[] sizeMax = new byte[268435456];
        //byte[] sizeMax = new byte[40960];
        Files.write(Paths.get(path), sizeMax);
    }
    /*
     http://www.botecodigital.info/java/gravando-e-lendo-dados-de-um-arquivo-em-java/
    http://examples.javacodegeeks.com/core-java/io/randomaccessfile/java-randomaccessfile-example/
    http://docs.oracle.com/javase/6/docs/api/java/io/RandomAccessFile.html
    
     */

    //write datablock in memory
    public void writeInDatablock(int numberOfDatablock, byte[] datas) throws IOException {
        try (RandomAccessFile write = new RandomAccessFile(path, "rw")) {
            write.seek(numberOfDatablock * 4096);
            write.write(datas);
            write.close();
        }catch (IOException ex) {
           System.err.print("ERRO AO ABRIR O ARQUIVO \n" + ex.getMessage());
        }     
    }

    //load datablock of memory
    public byte[] loadDatablock(int numberOfDatablock) throws IOException {
        try (RandomAccessFile read = new RandomAccessFile(path, "r")) {
            read.seek(numberOfDatablock * 4096);
            byte[] buf = new byte[4096];
            read.read(buf);
            read.close();
            return buf;
        }catch (IOException ex) {
           System.err.print("ERRO AO ABRIR O ARQUIVO \n" + ex.getMessage());
        } 
        return null;
    }
}
