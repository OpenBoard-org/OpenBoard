#include <QFile>
#include <QTextStream>

#include <stdlib.h>
#include <stdio.h>

#include <quazip/quachecksum32.h>
#include <quazip/quaadler32.h>
#include <quazip/quacrc32.h>

bool isSmallFile(const QFile &file)
{
	return file.size() < 1*1024*1024; // < 1 MB
}

int main(int argc, char* argv[])
{
	QTextStream console(stdout);

	if(argc != 2)
	{
		console << "usage: checksum filepath\n";
		return -1;
	}

	QFile file(argv[1]);
	if( !file.open(QFile::ReadOnly) )
	{
		console << "Coulden't open file " << argv[1] << '\n';
		return -2;
	}

	quint32 resoult;
	QuaChecksum32 *checksum = new QuaCrc32();// or QuaAdler32
	
	if(isSmallFile(file)) //then calculate the checksum in one step
	{
		resoult = checksum->calculate(file.readAll());
	}
	else //then calculate the checksum by streaming
	{
		const quint64 bufSize = 500*1024; //500 kB
		while(!file.atEnd())
		{
			checksum->update(file.read(bufSize));
		}
		resoult = checksum->value();
	}
	
	console << "checksum: " << hex << resoult << '\n';

	return 0;
}
