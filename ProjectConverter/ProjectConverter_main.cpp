/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#ifndef NATRON_BOOST_SERIALIZATION_COMPAT
#error "NATRON_BOOST_SERIALIZATION_COMPAT should be defined when compiling ProjectConverter to allow with projects older than Natron 2.2"
#endif

#include <string>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include "Engine/FStreamsSupport.h"

#include "Serialization/WorkspaceSerialization.h"
#include "Serialization/ProjectSerialization.h"
#include "Serialization/SerializationIO.h"

NATRON_NAMESPACE_USING

static void
printUsage(const std::string& programName)
{

                              /* Text must hold in 80 columns ************************************************/
    QString msg = QObject::tr("%1 usage:\n"
                              "This program can convert Natron projects (.ntp files) made with\n"
                              "Natron version 2.1.3 and older to the new project format used in 2.2.0.\n\n"
                              "Program options:\n\n"
                              "-i <filename>: Converts a .ntp (Project) or .nl (Workspace) file to\n"
                              "               a newer version. Upon failure an error message will be printed.\n"
                              "               In output, if the -o option is not passed, the converted\n"
                              "               file will have the same name as the input file with the\n"
                              "               \"converted\" extension before the file extension.\n\n"
                              "-i <dir name>: Converts all .ntp files under the given directory.\n\n"
                              "-r: If the path given to -i is a directory, recursively convert all recognized\n"
                              "    files in each sub-directory.\n\n"
                              "-o <filename/dirname>: Optional: If set this will instead output the\n"
                              "                       converted file or directory to this name instead.").arg(QString::fromUtf8(programName.c_str()));
    std::cout << msg.toStdString() << std::endl;
} // printUsage

static QStringList::iterator hasToken(QStringList &localArgs, const QString& token)
{
    for (QStringList::iterator it = localArgs.begin(); it!=localArgs.end(); ++it) {
        if (*it == token) {
            return it;
        }
    }
    return localArgs.end();
} // hasToken

static void parseArgs(const QStringList& appArgs, QString* inputPath, QString* outputPath, bool* recurse)
{
    QStringList localArgs = appArgs;
    {
        QStringList::iterator foundInput = hasToken(localArgs, QLatin1String("-i"));
        if (foundInput == localArgs.end()) {
            throw std::invalid_argument(QObject::tr("Missing -i switch").toStdString());
        } else {
            ++foundInput;
            if ( foundInput != localArgs.end() ) {
                *inputPath = *foundInput;
                localArgs.erase(foundInput);
            } else {
                throw std::invalid_argument(QObject::tr("-i switch without a file/directory name").toStdString());
            }
        }
    }
    {
        QStringList::iterator foundInput = hasToken(localArgs, QLatin1String("-o"));
        if (foundInput != localArgs.end()) {
            ++foundInput;
            if ( foundInput != localArgs.end() ) {
                *outputPath = *foundInput;
                localArgs.erase(foundInput);
            } else {
                throw std::invalid_argument(QObject::tr("-o switch without a file/directory name").toStdString());
            }
        }

    }
    *recurse = false;
    {
        QStringList::iterator foundInput = hasToken(localArgs, QLatin1String("-r"));
        if (foundInput != localArgs.end()) {
            if ( foundInput != localArgs.end() ) {
                *recurse = true;
            }
        }

    }
} // parseArgs



static void convertFile(const QString& filename, const QString& outputFileName)
{
    bool isProjectFile = filename.endsWith(QLatin1String(".ntp"));
    bool isWorkspaceFile = filename.endsWith(QLatin1String(".nl"));

    if (!isProjectFile && !isWorkspaceFile) {
        QString message = QObject::tr("%1 does not appear to be a .ntp or .nl file.");
        throw std::invalid_argument(message.toStdString());
    }

    QString outFileName = outputFileName;
    if (outFileName.isEmpty()) {
        int foundDot = filename.lastIndexOf(QLatin1Char('.'));
        if (foundDot != -1) {
            outFileName = filename.mid(0, foundDot);
            outFileName += QLatin1String("-converted.");
            outFileName += filename.mid(foundDot + 1);
        }
    }


    boost::shared_ptr<SERIALIZATION_NAMESPACE::ProjectSerialization> proj;
    boost::shared_ptr<SERIALIZATION_NAMESPACE::WorkspaceSerialization> workspace;
    // Read old file
    {
        FStreamsSupport::ifstream ifile;
        FStreamsSupport::open(&ifile, filename.toStdString());
        if (!ifile) {
            QString message = QObject::tr("Could not open %1").arg(filename);
            throw std::invalid_argument(message.toStdString());
        }

        if (isProjectFile) {
            proj.reset(new SERIALIZATION_NAMESPACE::ProjectSerialization);
            SERIALIZATION_NAMESPACE::tryReadAndConvertOlderProject(ifile, proj.get());
        } else if (isWorkspaceFile) {
            workspace.reset(new SERIALIZATION_NAMESPACE::WorkspaceSerialization);
            SERIALIZATION_NAMESPACE::tryReadAndConvertOlderWorkspace(ifile, workspace.get());
        }
    }

    // Write to converted file
    {
        FStreamsSupport::ofstream ofile;
        FStreamsSupport::open(&ofile, outFileName.toStdString());
        if (!ofile) {
            QString message = QObject::tr("Could not open %1").arg(outFileName);
            throw std::invalid_argument(message.toStdString());
        }

        if (proj) {
            SERIALIZATION_NAMESPACE::write(ofile, *proj);
        } else if (workspace) {
            SERIALIZATION_NAMESPACE::write(ofile, *workspace);
        }
    }
    
} // convertFile

static void convertDirectory(const QString& dirPath, const QString& outputDirPath, bool recurse, unsigned int recursionLevel)
{
    QDir d(dirPath);
    if (!d.exists()) {
        QString message = QObject::tr("%1: No such file or directory").arg(dirPath);
        throw std::invalid_argument(message.toStdString());
    }

    QString newDirPath = outputDirPath;
    if (outputDirPath.isEmpty()) {
        newDirPath = dirPath;
        if (recursionLevel == 0) {
            newDirPath += QLatin1String("_converted");
        }
    }

    QStringList files = d.entryList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    for (QStringList::const_iterator it = files.begin(); it!=files.end(); ++it) {

        QString absoluteFilePath = d.absolutePath() + QLatin1Char('/') + *it;
        QDir subDir(absoluteFilePath);
        if (subDir.exists()) {
            if (recurse) {
                QString childPath = newDirPath + QLatin1Char('/') + *it;
                convertDirectory(absoluteFilePath, childPath, recurse, recursionLevel + 1);
            }
            continue;
        }

        if (it->endsWith(QLatin1String(".ntp")) || it->endsWith(QLatin1String(".nl"))) {
            convertFile(absoluteFilePath, QString());
        }
    }
} // convertDirectory

int
main(int argc,
     char *argv[])
{
    QCoreApplication app(argc,argv);

    // Parse app args
    QString inputPath, outputPath;
    bool recurse;
    try {
        parseArgs(app.arguments(), & inputPath, &outputPath, &recurse);
    } catch (const std::exception &e) {
        std::cerr << QObject::tr("Error while parsing command line arguments: %1").arg(QString::fromUtf8(e.what())).toStdString() << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    QDir d(inputPath);
    if (d.exists()) {
        // This is a directory
        convertDirectory(inputPath, outputPath, recurse, 0);
    } else {
        if (!QFile::exists(inputPath)) {
            std::cerr << QObject::tr("%1: No such file or directory").arg(inputPath).toStdString() << std::endl;
            return 1;
        }
        try {
            convertFile(inputPath, outputPath);
        } catch (const std::exception& e) {
            std::cerr << QObject::tr("Error: %1").arg(QString::fromUtf8(e.what())).toStdString() << std::endl;
            return 1;
        }
    }
    return 0;
} // main
