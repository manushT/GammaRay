/*
  styleinjector.cpp

  This file is part of Endoscope, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config-endoscope.h>

#include "styleinjector.h"
#include "interactiveprocess.h"

#include <QProcess>
#include <cstdlib>

using namespace Endoscope;

StyleInjector::StyleInjector() :
  mExitCode(-1),
  mProcessError(QProcess::UnknownError),
  mExitStatus(QProcess::NormalExit)
{
}

bool StyleInjector::launch(const QStringList &programAndArgs,
                          const QString &probeDll, const QString &probeFunc)
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("ENDOSCOPE_STYLEINJECTOR_PROBEDLL", probeDll);
  env.insert("ENDOSCOPE_STYLEINJECTOR_PROBEFUNC", probeFunc);

  QString qtPluginPath = env.value("QT_PLUGIN_PATH");
  if (!qtPluginPath.isEmpty()) {
    qtPluginPath.append(":");
  }
  qtPluginPath.append(ENDOSCOPE_LIB_INSTALL_DIR "/qt4/plugins");
  env.insert("QT_PLUGIN_PATH", qtPluginPath);

  InteractiveProcess proc;
  proc.setProcessEnvironment(env);
  proc.setProcessChannelMode(QProcess::ForwardedChannels);

  QStringList args = programAndArgs;

  if (env.value("ENDOSCOPE_GDB").toInt()) {
    QStringList newArgs;
    newArgs << "gdb" << "--eval-command" << "run" << "--args";
    newArgs += args;
    args = newArgs;
  } else if (env.value("ENDOSCOPE_MEMCHECK").toInt()) {
    const QString tool = env.value("ENDOSCOPE_MEMCHECK");
    QStringList newArgs;
    newArgs << "valgrind" << "--tool=memcheck" << "--track-origins=yes" << "--num-callers=25";
    newArgs += args;
    args = newArgs;
  }

  const QString program = args.takeFirst();
  args << QLatin1String("-style") << QLatin1String("endoscope-injector");
  proc.start(program, args);
  proc.waitForFinished(-1);

  mExitCode = proc.exitCode();
  mProcessError = proc.error();
  mExitStatus = proc.exitStatus();
  mErrorString = proc.errorString();

  return mExitCode == EXIT_SUCCESS && mExitStatus == QProcess::NormalExit;
}

int StyleInjector::exitCode()
{
  return mExitCode;
}

QProcess::ProcessError StyleInjector::processError()
{
  return mProcessError;
}

QProcess::ExitStatus StyleInjector::exitStatus()
{
  return mExitStatus;
}

QString StyleInjector::errorString()
{
  return mErrorString;
}
