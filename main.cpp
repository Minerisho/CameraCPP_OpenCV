#include <opencv2/opencv.hpp>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>
using namespace std;
using namespace std::filesystem;




struct Resolution {
    int width;
    int height;
    string name;
};


std::string getOutputPath(const std::string& fileName) {

    path outputDir = current_path().parent_path() / "capturas";
    if (!exists(outputDir)) {
        create_directory(outputDir);
    }
    return (outputDir / fileName).string();
}

string getFileName(char m)
{
    time_t currentTime = time(0);
    tm* localTime = localtime(&currentTime);
    
    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1;
    int day = localTime->tm_mday;
    int hour = localTime->tm_hour;
    int minute = localTime->tm_min;
    int seconds = localTime->tm_sec;

    ostringstream oss;
    oss << (m == 'v' ? "VID" : "IMG") << year << (month < 10 ? "0" : "") << month 
        << (day < 10 ? "0" : "") << day 
        << (hour < 10 ? "0" : "") << hour 
        << (minute < 10 ? "0" : "") << minute 
        << (seconds < 10 ? "0" : "") << seconds 
        << (m == 'v' ? ".mp4" : ".jpg");

    return getOutputPath(oss.str()); 
}


int main()
{
    //Imprimir versión de C++}

    if (__cplusplus == 202002L) std::cout << "C++20\n";
    else if (__cplusplus == 201703L) std::cout << "C++17\n";
    else if (__cplusplus == 201402L) std::cout << "C++14\n";
    else if (__cplusplus == 201103L) std::cout << "C++11\n";
    else if (__cplusplus == 199711L) std::cout << "C++98\n";
    else std::cout << "pre-standard C++\n";

    cv::VideoCapture cam(0, cv::CAP_DSHOW); // quitar CAP_DSHOW después de copnfigurar la cámara
   
    if (!cam.isOpened())
    {
        cout << "Error: No se pudo abrir la cámara!" << endl;
        system("pause");
        return -1;
    }

    //Propiedades de imagen
    cam.set(cv::CAP_PROP_SETTINGS, 1); //Sólo funciona con CAP_DSHOW
    cam.set(cv::CAP_PROP_FPS, 30);
    cam.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, 1080); 


    cout << "Resolucion de: " << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "x" 
         << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << endl; //Resolución máxima alcanzada por la cámara si 1920x1080 no es soportado
    

    cv::Mat frame;
    bool recording = false;
    cv::VideoWriter videoWriter;

    cout << "\n###### Controles ########\n";
    cout << "g - Iniciar grabacion\n";
    cout << "d - Detener grabacion\n";
    cout << "c - Capturar imagen\n";
    cout << "q - Salir\n\n";

    while (true)
    {
        cam >> frame;
        
        char key = cv::waitKey(1);
        switch (key)
        {
            case 'q':
                goto exit_loop;  
                
            case 'g':
                if (!recording)
                {
                    string fileName = getFileName('v');
                    int frameWidth = cam.get(cv::CAP_PROP_FRAME_WIDTH);
                    int frameHeight = cam.get(cv::CAP_PROP_FRAME_HEIGHT);
                    
                    videoWriter.open(fileName, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, //poner 10 si se puso CAP_DSHOW, de lo contrario poner 30 (pienso agregar esta función automática más adelante)
                                    cv::Size(frameWidth, frameHeight));
                                    
                    if(!videoWriter.isOpened())
                    {
                        cout << "Error al abrir Video Writer" << endl;
                        videoWriter.release();
                    }
                    else 
                    {
                        recording = true;
                        cout << "Grabación iniciada -> " << fileName << endl;
                    }
                }
                break;
                
            case 'd':
                if (recording)
                {
                    videoWriter.release();
                    recording = false;
                    cout << "Grabación detenida" << endl;
                }
                break;
                
            case 'c':
                {
                    string fileName = getFileName('i');
                    if (cv::imwrite(fileName, frame))
                        cout << "Imagen guardada -> " << fileName << endl;
                    else
                        cout << "Error al guardar la imagen" << endl;
                }
                break;
        }
        
        if (recording)
        {
            videoWriter.write(frame);
            //Mostrar indicador visual de grabación
            cv::circle(frame, cv::Point(20, 20), 10, cv::Scalar(0, 0, 255), cv::FILLED);
            cv::putText(frame, "REC", cv::Point(40, 25), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }

        cv::imshow("Webcam", frame);
    }
    exit_loop:

    if (recording)
        videoWriter.release();
    cam.release();
    cv::destroyAllWindows();

    return 0;
}