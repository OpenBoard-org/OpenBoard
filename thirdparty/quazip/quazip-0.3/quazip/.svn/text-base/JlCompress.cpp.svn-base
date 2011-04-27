#include "JlCompress.h"
#include <QDebug>
/**OK
 * Comprime il file fileName, nell'oggetto zip, con il nome fileDest.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip è stato aperto in una modalità non compatibile con l'aggiunta di file;
 * * non è possibile aprire il file d'origine;
 * * non è possibile creare il file all'interno dell'oggetto zip;
 * * si è rilevato un errore nella copia dei dati;
 * * non è stato possibile chiudere il file all'interno dell'oggetto zip;
 */
bool JlCompress::compressFile(QuaZip* zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // fileName: nome del file reale
    // fileDest: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    // Apro il file originale
    QFile inFile;
    inFile.setFileName(fileName);
    if(!inFile.open(QIODevice::ReadOnly)) return false;

    // Apro il file risulato
    QuaZipFile outFile(zip);
    if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, inFile.fileName()))) return false;

    // Copio i dati
    char c;
    while(inFile.getChar(&c)&&outFile.putChar(c));
    if(outFile.getZipError()!=UNZ_OK) return false;

    // Chiudo i file
    outFile.close();
    if (outFile.getZipError()!=UNZ_OK) return false;
    inFile.close();

    return true;
}

/**OK
 * Comprime la cartella dir nel file fileCompressed, se recursive è true allora
 * comprime anche le sotto cartelle. I nomi dei file preceduti dal path creato
 * togliendo il pat della cartella origDir al path della cartella dir.
 * Se la funzione fallisce restituisce false e cancella il file che si è tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip è stato aperto in una modalità non compatibile con l'aggiunta di file;
 * * la cartella dir non esiste;
 * * la compressione di una sotto cartella fallisce (1);
 * * la compressione di un file fallisce;
 * (1) La funzione si richiama in maniera ricorsiva per comprimere le sotto cartelle
 * dunque gli errori di compressione di una sotto cartella sono gli stessi di questa
 * funzione.
 */
bool JlCompress::compressSubDir(QuaZip* zip, QString dir, QString origDir, bool recursive) {
    // zip: oggetto dove aggiungere il file
    // dir: cartella reale corrente
    // origDir: cartella reale originale
    // (path(dir)-path(origDir)) = path interno all'oggetto zip

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    // Controllo la cartella
    QDir directory(dir);
    if (!directory.exists()) return false;

    // Se comprimo anche le sotto cartelle
    if (recursive) {
        // Per ogni sotto cartella
        QFileInfoList files = directory.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot);
        foreach (QFileInfo file, files) {
            // Comprimo la sotto cartella
            if(!compressSubDir(zip,file.absoluteFilePath(),origDir,recursive)) return false;
        }
    }

    // Per ogni file nella cartella
    QFileInfoList files = directory.entryInfoList(QDir::Files);
    foreach (QFileInfo file, files) {
        // Se non è un file o è il file compresso che sto creando
        if(!file.isFile()||file.absoluteFilePath()==zip->getZipName()) continue;

        // Creo il nome relativo da usare all'interno del file compresso
        QString filename = file.absoluteFilePath().remove(QDir(origDir).absolutePath());

        // Comprimo il file
        if (!compressFile(zip,file.absoluteFilePath(),filename)) return false;
    }

    return true;
}

/**OK
 * Estrae il file fileName, contenuto nell'oggetto zip, con il nome fileDest.
 * Se la funzione fallisce restituisce false e cancella il file che si è tentato di estrarre.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip è stato aperto in una modalità non compatibile con l'estrazione di file;
 * * non è possibile aprire il file all'interno dell'oggetto zip;
 * * non è possibile creare il file estratto;
 * * si è rilevato un errore nella copia dei dati (1);
 * * non è stato possibile chiudere il file all'interno dell'oggetto zip (1);
 *
 * (1): prima di uscire dalla funzione cancella il file estratto.
 */
bool JlCompress::extractFile(QuaZip* zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdUnzip) return false;

    // Apro il file compresso
    zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if(!inFile.open(QIODevice::ReadOnly) || inFile.getZipError()!=UNZ_OK) return false;

    // Controllo esistenza cartella file risultato
    QDir().mkpath(QFileInfo(fileDest).absolutePath());

    // Apro il file risultato
    QFile outFile;
    outFile.setFileName(fileDest);
    if(!outFile.open(QIODevice::WriteOnly)) return false;

    // Copio i dati
    char c;
    while(inFile.getChar(&c)) outFile.putChar(c);
    if (inFile.getZipError()!=UNZ_OK) {
        removeFile(QStringList(fileDest));
        return false;
    }

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError()!=UNZ_OK) {
        removeFile(QStringList(fileDest));
        return false;
    }
    outFile.close();

    return true;
}

/**
 * Rimuove i file il cui nome è specificato all'interno di listFile.
 * Restituisce true se tutti i file sono stati cancellati correttamente, attenzione
 * perchè può restituire false anche se alcuni file non esistevano e si è tentato
 * di cancellarli.
 */
bool JlCompress::removeFile(QStringList listFile) {
    bool ret = true;
    // Per ogni file
    for (int i=0; i<listFile.count(); i++) {
        // Lo elimino
        ret = ret && QFile::remove(listFile.at(i));
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**OK
 * Comprime il file fileName nel file fileCompressed.
 * Se la funzione fallisce restituisce false e cancella il file che si è tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione del file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool JlCompress::compressFile(QString fileCompressed, QString file) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Aggiungo il file
    if (!compressFile(zip,file,QFileInfo(file).fileName())) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }
    delete zip;

    return true;
}

/**OK
 * Comprime i file specificati in files nel file fileCompressed.
 * Se la funzione fallisce restituisce false e cancella il file che si è tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool JlCompress::compressFiles(QString fileCompressed, QStringList files) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Comprimo i file
    QFileInfo info;
    foreach (QString file, files) {
        info.setFile(file);
        if (!info.exists() || !compressFile(zip,file,info.fileName())) {
            delete zip;
            QFile::remove(fileCompressed);
            return false;
        }
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }
    delete zip;

    return true;
}

/**OK
 * Comprime la cartella dir nel file fileCompressed, se recursive è true allora
 * comprime anche le sotto cartelle.
 * Se la funzione fallisce restituisce false e cancella il file che si è tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool JlCompress::compressDir(QString fileCompressed, QString dir, bool recursive) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Aggiungo i file e le sotto cartelle
    if (!compressSubDir(zip,dir,dir,recursive)<0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }
    delete zip;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**OK
 * Estrae il file fileName, contenuto nel file fileCompressed, con il nome fileDest.
 * Se fileDest = "" allora il file viene estratto con lo stesso nome con cui è
 * stato compresso.
 * Se la funzione fallisce cancella il file che si è tentato di estrarre.
 * Restituisce il nome assoluto del file estratto.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * l'estrazione del file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QString JlCompress::extractFile(QString fileCompressed, QString fileName, QString fileDest) {
    // Apro lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QString();
    }

    // Estraggo il file
    if (fileDest.isEmpty()) fileDest = fileName;
    if (!extractFile(zip,fileName,fileDest)) {
        delete zip;
        return QString();
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        removeFile(QStringList(fileDest));
        return QString();
    }
    delete zip;

    return QFileInfo(fileDest).absoluteFilePath();
}

/**OK
 * Estrae i file specificati in files, contenuti nel file fileCompressed, nella
 * cartella dir. La struttura a cartelle del file compresso viene rispettata.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si è tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * l'estrazione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList JlCompress::extractFiles(QString fileCompressed, QStringList files, QString dir) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Estraggo i file
    for (int i=0; i<files.count(); i++) {
        if (!extractFile(zip, files.at(i), QDir(dir).absoluteFilePath(files.at(i)))) {
            delete zip;
            removeFile(files);
            return QStringList();
        }
        files[i] = QDir(dir).absoluteFilePath(files.at(i));
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        removeFile(files);
        return QStringList();
    }
    delete zip;

    return files;
}

/**OK
 * Estrae il file fileCompressed nella cartella dir.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si è tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList JlCompress::extractDir(QString fileCompressed, QString dir) {
    // Apro lo zip
    QuaZip* zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Estraggo i file
    QStringList lst = getFileList(fileCompressed);

    for (int i=0; i<lst.count(); i++) {
        if (!extractFile(zip, lst.at(i), QDir(dir).absolutePath() + lst.at(i))) {
            delete zip;
            removeFile(lst);
            return QStringList();
        }
        lst[i] = QDir(dir).absolutePath() + lst.at(i);
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        removeFile(lst);
        return QStringList();
    }
    delete zip;

    return lst;
}

/**OK
 * Restituisce la lista dei file resenti nel file compresso fileCompressed.
 * Se la funzione fallisce, restituisce un elenco vuoto.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la richiesta di informazioni di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList JlCompress::getFileList(QString fileCompressed) {
    // Apro lo zip
    QuaZip* zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Estraggo i nomi dei file
    QStringList lst;
    QuaZipFileInfo info;
    for(bool more=zip->goToFirstFile(); more; more=zip->goToNextFile()) {
      if(!zip->getCurrentFileInfo(&info)) {
          delete zip;
          return QStringList();
      }
      lst << info.name;
      //info.name.toLocal8Bit().constData()
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        return QStringList();
    }
    delete zip;

    return lst;
}

