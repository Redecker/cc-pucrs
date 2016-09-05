package simuladordebanco;

import java.io.IOException;
import java.util.Scanner;

/**
 *
 * @author Arthur
 * @author Matheus
 * @author Thomas
 */
public class main {
    public static void main(String[] args) throws IOException {
        
        
        Controller control = new Controller();
        control.resetDataBase();
        
//        
//        for(int i = 0; i < 55 ;i++){
//            System.out.println(control.seekSerial(i));
//        }
        
//        byte[] text1 = db.searchReturnData((short)1);
//        // to transform in string
//        String theString = new String(text1, "ASCII"); 
//        
//        System.out.println(theString);
//        
//        text1 = db.searchReturnData((short) 2);
//        
//        theString = new String(text1, "ASCII"); 
//        System.out.println(theString);
//        
//        for (int i = 0; i < test.length ;i++){
//            System.out.println(Byte.toString(test[i]));
//        }
        
        //for(int i = 0; i < dr.getData().length ; i++) System.out.println(Integer.toBinaryString(0xFF & dr.getData()[i]));
        
//        // TODO code application logic here
//        Buffer buffer = new Buffer();
//        DataFile dataFile = new DataFile();
        
        Scanner scanner = new Scanner(System.in);
        String tecla;
        boolean sair = false;
        while (!sair) {
            menu();
            tecla = scanner.nextLine();
            switch (tecla) {
                case "1":
                    while (true) {
                        opcao1();
                        tecla = scanner.nextLine();
                        if(tecla.equals("1")){                            
                            System.out.println("Digite o texto que desejar");
                            String text = scanner.nextLine();
                            control.addNewElement(text);
                        }
                        if(tecla.equals("2")){
                            System.out.println("Digite o nome do arquivo .json");
                            String path = scanner.nextLine();
                            control.addNewElementJson(path);
                        }
                        if (tecla.equals("9")) {
                            break;
                        }
                    }
                    break;
                case "2":
                    while (true) {
                        opcao2();
                        tecla = scanner.nextLine();
                        if(tecla.equals("1")){                
                            System.out.println("Digite o ID que deseja virzualizar: ");
                            int value = scanner.nextInt();
                            String data = control.seekSerialReturnData(value);
                            System.out.println(data);
                        }
                        if(tecla.equals("2")){
                            System.out.println("Não implementado.");
                        }
                        if (tecla.equals("9")) {
                            break;
                        }
                    }
                    break;
                case "4":
                    while (true) {
                        opcao4();
                        tecla = scanner.nextLine();
                        if(tecla.equals("1")){
                            System.out.println("Digite o ID que deseja excluir: ");
                            int id = scanner.nextInt();
                            if(control.seekDeleteSerial(id)){
                                System.out.println("Deletado com sucesso");
                            }else{
                                System.out.println("ID inexistente");
                            }
                        }
                        if (tecla.equals("9")) {
                            break;
                        }
                    }
                    break;
                case "5":
                    while (true) {
                        opcao5();
                        tecla = scanner.nextLine();
                        System.out.println("Não implementado");
                        if(tecla.equals("1")){
                            System.out.println("Digite o nome de um .txt com todos os nomes dos arquivos .json");
                            String jsons = scanner.nextLine();
                            control.loadLots(jsons);                            
                        }
                            
                        if (tecla.equals("9")) {
                            break;
                        }
                    }
                    break;
                case "9":
                    sair = true;
                    control.saveFramesWhenClose();
                    break;
            }
        }
    }

    private static void menu() {
        System.out.println("       SIMULADOR DE BANCO 1.0");
        System.out.println("---------------------------------------");
        System.out.println("|Selecione a opção desejada:          |");
        System.out.println("|1 - Inserir                          |");
        System.out.println("|2 - Buscar                           |");
        System.out.println("|4 - Excluir                          |");
        System.out.println("|5 - Carregar                         |");
        System.out.println("|9 - Sair                             |");
        System.out.println("---------------------------------------");

    }

    private static void opcao1() {
        System.out.println("|Menu Inserir                         |");
        System.out.println("|1 - Inserir Digitando                |");
        System.out.println("|2 - Inserir arquivo json             |");
        System.out.println("|9 - Voltar                           |");

    }

    private static void opcao2() {
        System.out.println("|Menu Buscar                          |");
        System.out.println("|1 - Buscar por ID                    |");
        System.out.println("|2 - Buscar por tag                   |");
        System.out.println("|9 - Voltar                           |");

    }

    private static void opcao4() {
        System.out.println("|Menu Excluir                         |");
        System.out.println("|1 - Excluir por ID                   |");
        System.out.println("|9 - Voltar                           |");

    }

    private static void opcao5() {
        System.out.println("|Menu Carregar                        |");
        System.out.println("|1 - Carregar em lote                 |");
        System.out.println("|9 - Voltar                           |");

    }

}
