package simuladordebanco;

public class DataBlock {

    // in bytes -- 1byte = 8 bits = 0xXX  
    private final int size = 4096;
    public int headerPointer;
    public int datasPointer;
    private byte[] serialize;

    //if is a new datablock
    public DataBlock() {
        headerPointer = 0;
        datasPointer = size - 1;
        serialize = new byte[size];
    }

    //if came of memory
    public DataBlock(byte[] serialize) {
        this.serialize = serialize;
        int[] pointers = findPointer();
        headerPointer = pointers[0];
        datasPointer = pointers[1];
    }
    
    // find the pointers of a datablock read of disk
    private int[] findPointer() {
         
        int initialAux = 4095;
        int header = 0;
        
        
        for (int i = 0; i < 4096; i++) {
            String initialBinary = Integer.toBinaryString(0xFF & serialize[i * 4]) + Integer.toBinaryString(0xFF & serialize[i * 4 + 1]);
            short initial = Short.parseShort(initialBinary, 2);
            String sizeBinary = Integer.toBinaryString(0xFF & serialize[i * 4 + 2]) + Integer.toBinaryString(0xFF & serialize[i * 4 + 3]);
            short sizeData = Short.parseShort(sizeBinary, 2);
            
            if (initial == 0 && sizeData == 0) {
                header = i*4;
                break;
            }
            //get the higher initial 
            if (initialAux > initial){
                initialAux = initial;
            }
            
        }

        int[] toReturn = new int[2];
        
        toReturn[0] = header;
        toReturn[1] = initialAux-1;
        
        return toReturn;
    }
    
    //last address used
    public int getAddressDatablock(){
        return headerPointer/4 - 1;
    }
    
    //get byte[] of datablock
    public byte[] getDataBlock() {
        return serialize;
    }
    
    // size avaiable in datablock
    public int getSizeLeft() {
        return datasPointer - headerPointer;
    }

    //add a data in datablock
    public boolean addData(int id, String json) {
        DataRecord dataRec = new DataRecord(id, json);
        byte[] data = dataRec.getData();

        HeaderEntry headEnt = new HeaderEntry((short) (datasPointer - data.length + 1), (short) data.length);

        return addData(headEnt, dataRec);
    }

    private boolean addData(HeaderEntry he, DataRecord dr) {

        //if the left size in datablock is higher than 4(headerEntry) + dataSize()
        //you can add if pass 
        if (getSizeLeft() > (4 + dr.getData().length)) {

            //write the header
            byte[] headerEntry = he.getHeader();

            serialize[headerPointer] = headerEntry[0];
            serialize[headerPointer + 1] = headerEntry[1];
            serialize[headerPointer + 2] = headerEntry[2];
            serialize[headerPointer + 3] = headerEntry[3];

            headerPointer += 4;
            
            //write the datas
            byte[] dataRecord = dr.getData();

            int sum = 0;
            for (int i = dataRecord.length - 1; i >= 0; i--) {
                serialize[datasPointer - sum] = dataRecord[i];
                sum++;
            }

            datasPointer -= sum;
            return true;
        }
        return false;
    }

    // set a "hole" in the datablock that never comeback FIX IT! make a list of hole places or 
    // compress all the other datas
    // imply in change initial in header
    public void delete(short addressInDataBlock) {
       
        //transform 2 bytes in 1 short        
        String initialBinary = Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4]) + Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4 + 1]);
        short initial = Short.parseShort(initialBinary, 2);

        String sizeBinary = Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4 + 2]) + Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4 + 3]);
        short sizeData = Short.parseShort(sizeBinary, 2);

        //delete the data
        serialize[addressInDataBlock * 4] = 0x00;
        serialize[addressInDataBlock * 4 + 1] = 0x00;
        serialize[addressInDataBlock * 4 + 2] = 0x00;
        serialize[addressInDataBlock * 4 + 3] = 0x00;

        for (short i = initial; i < initial + sizeData; i++) {
            serialize[i] = 0x00;
        }
    }
    
    // addres in header entry return data
    public byte[] searchReturnData(short addressInDataBlock) {
        //take initial and size binary from memory
        String initialBinary = Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4]) + Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4 + 1]);
        short initial = Short.parseShort(initialBinary, 2);

        String sizeBinary = Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4 + 2]) + Integer.toBinaryString(0xFF & serialize[addressInDataBlock * 4 + 3]);
        short sizeData = Short.parseShort(sizeBinary, 2);

        //if 0 no have data
        if (initial == 0 && sizeData == 0) {
            return null;
        }

        //return data
        byte[] toReturn = new byte[sizeData];
        
        int j = 0;
        for (short i = initial; i < initial + sizeData; i++) {
            toReturn[j] = serialize[i];
            j++;
        }

        return toReturn;
    }
}