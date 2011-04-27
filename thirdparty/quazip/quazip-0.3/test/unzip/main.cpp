#include <QDir>
#include <QFile>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

/* A simple test program. Requires "test.zip" and writable "out"
 * directory to be present in the current directory.
 *
 * To test unicode-aware case sensitivity, see testCase() function.
 */

// test reading archive
bool testRead()
{
  QuaZip zip("test.zip");
  if(!zip.open(QuaZip::mdUnzip)) {
    qWarning("testRead(): zip.open(): %d", zip.getZipError());
    return false;
  }
  zip.setFileNameCodec("IBM866");
  printf("%d entries\n", zip.getEntriesCount());
  printf("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());
  QuaZipFileInfo info;
  printf("name\tcver\tnver\tflags\tmethod\tctime\tCRC\tcsize\tusize\tdisknum\tIA\tEA\tcomment\textra\n");
  QuaZipFile file(&zip);
  QFile out;
  QString name;
  char c;
  for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
    if(!zip.getCurrentFileInfo(&info)) {
      qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
      return false;
    }
    printf("%s\t%hu\t%hu\t%hu\t%hu\t%s\t%u\t%u\t%u\t%hu\t%hu\t%u\t%s\t%s\n",
        info.name.toLocal8Bit().constData(),
        info.versionCreated, info.versionNeeded, info.flags, info.method,
        info.dateTime.toString(Qt::ISODate).toLocal8Bit().constData(),
        info.crc, info.compressedSize, info.uncompressedSize, info.diskNumberStart,
        info.internalAttr, info.externalAttr,
        info.comment.toLocal8Bit().constData(), info.extra.constData());
    if(!file.open(QIODevice::ReadOnly)) {
      qWarning("testRead(): file.open(): %d", file.getZipError());
      return false;
    }
    name=file.getActualFileName();
    if(file.getZipError()!=UNZ_OK) {
      qWarning("testRead(): file.getFileName(): %d", file.getZipError());
      return false;
    }
    QString dirn = "out/" + name;
    if (name.contains('/')) { // subdirectory support
      // there must be a more elegant way of doing this
      // but I couldn't find anything useful in QDir
      dirn.chop(dirn.length() - dirn.lastIndexOf("/"));
      QDir().mkpath(dirn);
    }
    out.setFileName("out/" + name);
    out.open(QIODevice::WriteOnly);
    char buf[4096];
    int len = 0;
    while (file.getChar(&c)) {
      // we could just do this, but it's about 40% slower:
      // out.putChar(c);
      buf[len++] = c;
      if (len >= 4096) {
        out.write(buf, len);
        len = 0;
      }
    }
    if (len > 0) {
      out.write(buf, len);
    }
    out.close();
    if(file.getZipError()!=UNZ_OK) {
      qWarning("testRead(): file.getFileName(): %d", file.getZipError());
      return false;
    }
    if(!file.atEnd()) {
      qWarning("testRead(): read all but not EOF");
      return false;
    }
    file.close();
    if(file.getZipError()!=UNZ_OK) {
      qWarning("testRead(): file.close(): %d", file.getZipError());
      return false;
    }
  }
  zip.close();
  if(zip.getZipError()!=UNZ_OK) {
    qWarning("testRead(): zip.close(): %d", zip.getZipError());
    return false;
  }
  return true;
}

// test pos(), size(), csize(), usize(), ungetChar(), bytesAvailable()
bool testPos()
{
  QuaZip zip("test.zip");
  if(!zip.open(QuaZip::mdUnzip)) {
    qWarning("testPos(): zip.open(): %d", zip.getZipError());
    return false;
  }
  if(!zip.goToFirstFile()) {
    qWarning("testPos(): zip.goToFirstFile(): %d", zip.getZipError());
    return false;
  }
  QuaZipFile file(&zip);
  int method;
  if(!file.open(QIODevice::ReadOnly, &method, NULL, true)) {
    qWarning("testPos(): file.open(raw): %d", file.getZipError());
    return false;
  }
  QByteArray array=file.readAll();
  if(array.isEmpty()) {
    qWarning("testPos(): file.readAll(): %d", file.getZipError());
    return false;
  }
  qint64 pos=file.pos();
  if(pos!=file.size()||file.size()!=file.csize()) {
    qWarning("testPos(): pos=%Ld, file.size()=%Ld, file.csize()=%Ld", pos, file.size(), file.csize());
    return false;
  }
  char last=array.at(array.size()-1);
  file.ungetChar(last);
  char next;
  if(!file.getChar(&next)) {
    qWarning("testPos(): file.getChar(): %d", file.getZipError());
    return false;
  }
  if(last!=next) {
    qWarning("testPos(): ungot %d, got %d", (int)(uchar)last, (int)(uchar)next);
    return false;
  }
  if(file.pos()!=pos) { // position should not change
    qWarning("testPos(): position changed: old pos=%Ld, new pos=%Ld", pos, file.pos());
    return false;
  }
  file.close();
  if(zip.getZipError()!=UNZ_OK) {
    qWarning("testPos(): file.close(raw): %d", file.getZipError());
    return false;
  }
  if(!file.open(QIODevice::ReadOnly, &method, NULL, false)) {
    qWarning("testPos(): file.open(): %d", file.getZipError());
    return false;
  }
  array=file.readAll();
  pos=file.pos();
  if(pos!=file.size()||file.size()!=file.usize()) {
    qWarning("testPos(): pos=%Ld, file.size()=%Ld, file.usize()=%Ld", pos, file.size(), file.usize());
    return false;
  }
  file.close();
  if(zip.getZipError()!=UNZ_OK) {
    qWarning("testPos(): file.close(): %d", file.getZipError());
    return false;
  }
  zip.close();
  if(zip.getZipError()!=UNZ_OK) {
    qWarning("testPos(): zip.close(): %d", zip.getZipError());
    return false;
  }
  return true;
}

// test unicode-aware case sensitivity
// change the name and file name codec below before compiling
bool testCase()
{
  QString name=QString::fromUtf8("01_КАФЕ НА ТРОТУАРЕ.OGG");
  QuaZip zip("test.zip");
  if(!zip.open(QuaZip::mdUnzip)) {
    qWarning("testCase(): zip.open(): %d", zip.getZipError());
    return false;
  }
  zip.setFileNameCodec("IBM866");
  if(!zip.setCurrentFile(name, QuaZip::csInsensitive)) {
    if(zip.getZipError()==UNZ_OK)
      qWarning("testCase(): setCurrentFile(): check the file name");
    else
      qWarning("testCase(): setCurrentFile(): %d", zip.getZipError());
    return false;
  }
  if(zip.setCurrentFile(name, QuaZip::csSensitive)) {
    qWarning("testCase(): setCurrentFile(): sets even if the case is wrong");
    return false;
  }
  zip.close();
  return true;
}

int main()
{
  if(!testRead()) return 1;
  if(!testPos()) return 1;
  if(!testCase()) return 1;
  return 0;
}
