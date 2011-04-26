#include <QtCore>

#include <quazip/JlCompress.h>
#include <quazip/quazip.h>

int main() {
    QStringList lst;

    ////////////////
    // Compression
    qDebug() << "Compression:";

    // Compress a single file
    qDebug() << "Compression of main.cpp in tmp/example1.zip:" \
             << JlCompress::compressFile("tmp/example1.zip","main.cpp");

    // Compress a list of files
    lst.clear();
    lst << "main.cpp" << "Makefile" << "jlcompress.pro";
    qDebug() << "Compression of " << lst << " in tmp/example2.zip: " \
             << JlCompress::compressFiles("tmp/example2.zip",lst);

    // Compress a dir (current dir)
    qDebug() << "Compression of all files and subdir in tmp/example3.zip:" \
             << JlCompress::compressDir("tmp/example3.zip");


    ///////////////
    // Extraction
    qDebug() << "\n\nExtraction:";

    // Print file's names in the compressed file
    qDebug() << "Files in tmp/example3.zip (all):";
    qDebug() << JlCompress::getFileList("tmp/example3.zip");

    // Extract a file
    qDebug() << "\nFile extract from tmp/example1.zip (main.cpp):";
    qDebug() << JlCompress::extractFile("tmp/example1.zip","main.cpp","tmp/ext1/main.cpp");

    // Extract a list of files
    lst.clear();
    lst << "main.cpp" << "Makefile";
    qDebug() << "\nFiles extract from tmp/example2.zip " << lst << ":";
    qDebug() << JlCompress::extractFiles("tmp/example2.zip",lst,"tmp/ext2");

    // Extract a dir
    qDebug() << "\nFiles extracted from tmp/example3.zip (all):";
    qDebug() << JlCompress::extractDir("tmp/example3.zip","tmp/ext3");
    return 0;
}


/*
int compressFolder(QString folder, QString filename, bool subfolders = true) {
    // Se la cartella non esiste o non è una cartella
    if (!QFileInfo(folder).isDir()) return -ERR_IS_NOT_DIRECTORY;

    // Pero goni file presente nella cartella
    QFileInfoList files = QDir(folder).entryInfoList();
    QFile inFile;
    foreach(QFileInfo file, files) {
        // Controllare se il file è lo stesso che si crea
        if (file==QFile(filename)) continue;

        // Se è un file
        if (file.isFile()) {
            // Apro il file
            inFile.setFileName(file.fileName());
            if(!inFile.open(QIODevice::ReadOnly)) return -ERR_CANT_OPEN_FILE;

        }

        // Se è una sotto cartella
        if (file.isDir() && subfolders) {

        }
    }


      QFile inFile;
      QuaZipFile outFile(&zip);
      char c;
      foreach(QFileInfo file, files) {
        if(!file.isFile()||file.fileName()=="test.zip") continue;
        inFile.setFileName(file.fileName());
        if(!inFile.open(QIODevice::ReadOnly)) {
          qWarning("testCreate(): inFile.open(): %s", inFile.errorString().toLocal8Bit().constData());
          return false;
        }
        if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(inFile.fileName(), inFile.fileName()))) {
          qWarning("testCreate(): outFile.open(): %d", outFile.getZipError());
          return false;
        }
        while(inFile.getChar(&c)&&outFile.putChar(c));
        if(outFile.getZipError()!=UNZ_OK) {
          qWarning("testCreate(): outFile.putChar(): %d", outFile.getZipError());
          return false;
        }
        outFile.close();
        if(outFile.getZipError()!=UNZ_OK) {
          qWarning("testCreate(): outFile.close(): %d", outFile.getZipError());
          return false;
        }
        inFile.close();
      }

}
*/
