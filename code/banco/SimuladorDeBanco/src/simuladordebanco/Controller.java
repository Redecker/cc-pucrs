package simuladordebanco;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.util.ArrayList;

public class Controller {

    private DataFile dataFile;
    public int validDatablock;
    private static int nextID;

    public Controller() throws IOException {

        dataFile = new DataFile();
        validDatablock = 0;
        nextID = 1;        
        //loadNeededInformations();
    }   
    
    //did not working
//    private void saveNeededInformations() throws IOException {
//        byte[] validDataBlockToSave = intToByteArray(validDatablock+1);
//        dataFile.writeInDatablock(1, validDataBlockToSave);
//        
//        byte[] nextIDToSave = intToByteArray(nextID);
//        dataFile.writeInDatablock(2, nextIDToSave);
//        
//        try (BufferedWriter writer = new BufferedWriter(new FileWriter("btree.txt", true))){
//	    // get the lineBreak 
//	    String lineBreak = System.getProperty("line.separator");
//
//	    // writes UserName + the Tweet + location in the file
//	    
//                        
//            for(int i = 0 ; i < btreeSerial.size() ; i++){
//                writer.append(btreeSerial.get(i)+"|"+Datablock.get(i)+"|"+addressDatablock.get(i)+lineBreak);  
//            }           
//                    
//	    // close the file
//	    writer.close();  
//	    } catch (IOException x) {
//	        System.err.format("FAIL ON WRITE IN THE FILE, ERROR: ", x);
//            }
//    }
    
    //did not working
    //when initialize controller load all informations
//    private void loadNeededInformations() throws IOException {
//        //read datablock with informations about next valid datablock and next id
//        byte[] validOnDisk = readDataBlockOnDisk(1);
//        byte[] validToReturn = new byte[4];
//        validToReturn[0] = validOnDisk[0];
//        validToReturn[1] = validOnDisk[1];
//        validToReturn[2] = validOnDisk[2];
//        validToReturn[3] = validOnDisk[3];
//        validDatablock = byteArrayToInt(validToReturn);
//
//        byte[] nextIDOnDisk = readDataBlockOnDisk(2);
//        byte[] nextIDToReturn = new byte[4];
//        nextIDToReturn[0] = nextIDOnDisk[0];
//        nextIDToReturn[1] = nextIDOnDisk[1];
//        nextIDToReturn[2] = nextIDOnDisk[2];
//        nextIDToReturn[3] = nextIDOnDisk[3];
//        nextID = byteArrayToInt(nextIDToReturn);
//        
//        String dataJson = "";		
//        try (BufferedReader buffRead = new BufferedReader(new InputStreamReader(new FileInputStream("btree.txt"), "UTF8"))) {
//            String lineRead = "";			
//	    while(buffRead.ready()) { 
//                lineRead = buffRead.readLine();
//                String[] line = lineRead.split("|");
//                btreeSerial.add(Integer.getInteger(line[0]));
//                Datablock.add(Integer.getInteger(line[1]));
//                addressDatablock.add(Integer.getInteger(line[2]));
//		} 
//            buffRead.close();				
//	    } catch (Exception e) {
//	    	e.printStackTrace();
//                }        
//        
//    }

    private int byteArrayToInt(byte[] b) {
        return b[3] & 0xFF
                | (b[2] & 0xFF) << 8
                | (b[1] & 0xFF) << 16
                | (b[0] & 0xFF) << 24;
    }

    private byte[] intToByteArray(int a) {
        return new byte[]{
            (byte) ((a >> 24) & 0xFF),
            (byte) ((a >> 16) & 0xFF),
            (byte) ((a >> 8) & 0xFF),
            (byte) (a & 0xFF)
        };
    }
    
    //BTREE    
    private ArrayList<Integer> btreeSerial = new ArrayList<>();
    private ArrayList<Integer> Datablock= new ArrayList<>();
    private ArrayList<Integer> addressDatablock= new ArrayList<>();
    
    //search for ID and return the data of that register    
    public String seekSerialReturnData(int serial) throws IOException{
        
        int index = btreeSerial.indexOf(serial);
        if(index != -1){
            //find datablock in btree
            int datablock = Datablock.get(index);
            int addressdatablock = addressDatablock.get(index);
            DataBlock wanted = readDataBlockBuff(datablock);
            //take the data and return in string
            byte[] datas = wanted.searchReturnData((short)addressdatablock);
            byte[] dataWithoutSerial = new byte[datas.length-4];
            int j = 0;
            for(int i = 4; i < datas.length ; i++){
                dataWithoutSerial[j] = datas[i];
                j++;
            }
            
            String theString = new String(dataWithoutSerial, "ASCII"); 
            return theString;
        }
        return "NÃO EXISTE ESSE ID SERIAL";
    }
    //search for a ID and delete the data
    public boolean seekDeleteSerial(int serial) throws IOException{
        
        int index = btreeSerial.indexOf(serial);
        if(index != -1){
            //take the datablock in btree
            int datablock = Datablock.get(index);
            int addressdatablock = addressDatablock.get(index);
            
            //remove from btree
            btreeSerial.remove(index);
            Datablock.remove(index);
            addressDatablock.remove(index);
            
            DataBlock wanted = readDataBlockBuff(datablock);
            
            //delete in datablock
            wanted.delete((short) addressdatablock);
            return true;
        } 
        return false;
    }
    
    //load .txt with paths of json archives
    public void loadLots(String path){		
            try (BufferedReader buffRead = new BufferedReader(new InputStreamReader(new FileInputStream(path), "UTF8"))) {
		String lineRead = "";			
		while(buffRead.ready()) { 
                    lineRead = buffRead.readLine();
                    addNewElementJson(lineRead);
		} 
		buffRead.close();				
	    } catch (Exception e) {
	    	e.printStackTrace();
                }
    }

    //add a json file
    public void addNewElementJson(String path) throws IOException{
        //open json get the string and return 
            String dataJson = "";		
            try (BufferedReader buffRead = new BufferedReader(new InputStreamReader(new FileInputStream(path), "UTF8"))) {
		String lineRead = "";			
		while(buffRead.ready()) { 
                    lineRead = buffRead.readLine();
                    dataJson += lineRead;                    
		} 
		buffRead.close();				
	    } catch (Exception e) {
	    	e.printStackTrace();
                }
            
            addNewElement(dataJson);
    }
    
    //add new element in datablock avaiable
    public void addNewElement(String value) throws IOException{
        DataBlock toSave = readDataBlockBuff(validDatablock);
        
        if (!toSave.addData(nextID, value)){
            //if cannot add //no space // get the next datablock and call again
            validDatablock ++;
            addNewElement(value);
        }else{
            //add with sucess increment the next ID
            btreeSerial.add(nextID);
            Datablock.add(validDatablock);
            addressDatablock.add(toSave.getAddressDatablock());
            nextID++;
        }
    }
    
    //DATAFILE
    // read datablock and return a datablock!
    private DataBlock readDataBlock(int numberOfDB) throws IOException{
        return new DataBlock(readDataBlockOnDisk(numberOfDB));
    }    
    // save datablock in disk
    private void saveDataBlockInDisk(DataBlock db, int pos) throws IOException {
        dataFile.writeInDatablock(pos, db.getDataBlock());
    }
    // read datablock in disk
    private byte[] readDataBlockOnDisk(int dataBlockNumber) throws IOException {
        return dataFile.loadDatablock(dataBlockNumber);
    }
        
    //reset database
    public void resetDataBase() throws IOException{
        dataFile.createBinary();
    }
    
    
    //BUFFER
    private ArrayList<DataBlock> frames = new ArrayList<>();
    private ArrayList<Boolean> clock = new ArrayList<>();
    private ArrayList<Integer> numberDataBlockInFrame = new ArrayList<>();
    private int pointer = 0;
    private int pointerFree = 0;
    private DataBlock addBuffer(DataBlock db, int number) throws IOException{
        if(frames.size() == 256){
            //buff is full
            //what index in buffer have to change
            int toDelete = clock();
            //save the old datablock in disk
            saveDataBlockInDisk(frames.get(toDelete), numberDataBlockInFrame.get(toDelete));
            //read the new datablock to save
            DataBlock neww = readDataBlock(number);
            //set in buffer and for the clock algorithm
            frames.set(toDelete, neww);
            clock.set(toDelete, true);
            numberDataBlockInFrame.set(toDelete, number);
            //to set the pointer
            if(toDelete == 255){pointer = 0; return frames.get(255);} else{pointer = toDelete + 1;}
            //return the new file
            return frames.get(pointer-1);            
        }else{
            //if buffer is not full
            //add in frames set clock as false save the number of datablock
            //return the new one
            frames.add(db);
            clock.add(false);
            numberDataBlockInFrame.add(number);
            pointerFree++;
            return frames.get(pointerFree-1);
        }
    }
    // read datablock through the buffer
    public DataBlock readDataBlockBuff(int dataBlock) throws IOException { 
        DataBlock inBuff = alreadyInBuff(dataBlock);
        if(inBuff != null) {
            
            return inBuff;
        }
        return addBuffer(readDataBlock(dataBlock), dataBlock);
    }   
   
    private int clock() {
        int toReturn = -1;
        for (int i = pointer; i < 256; i++) {
            // if is '0' in clock algorithm 
            if (clock.get(i) == false) {
                return i;
            } else {
                // make the '1' become '0' in clock algorithm
                clock.set(pointer,false);                
            }
            // go to the start of the list
            if (pointer == 255) {
                pointer = 0;
            }
        }
        // if didnt find any. Its cannot happen
        return toReturn;
    }
    
    //see if the datablock is in buffer
    private DataBlock alreadyInBuff(int numDataBlock){
        for(int i = 0 ; i < frames.size() ; i++){
            if(numberDataBlockInFrame.get(i) == numDataBlock){
                //set that they have visit
                clock.set(i,true);
                return frames.get(i);
            }
        }
        return null;
    }
    
    //save buffer in disk when close
    public void saveFramesWhenClose() throws IOException{
        for(int i = 0 ; i < frames.size() ; i++){
            saveDataBlockInDisk(frames.get(i), numberDataBlockInFrame.get(i));
        }
        //saveNeededInformations();
    }

    
}
