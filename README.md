# TAR32.DLL
[Common Archivers Project](http://www.csdinc.co.jp/archiver/)

## はじめに
TAR32.DLLはWindows用のTAR/GZ/BZ2/Z等の圧縮/展開用ライブラリプログラムです。
統合アーカイバプロジェクトのAPI仕様に準拠しています。
対応形式は以下の通りです。
 - .gz (gzip format)
 - .Z (compress utility format) / extrace only
 - .bz2 (bzip2 format)
 - .tar (Tape ARchiver format)
 - .cpio (cpio archive format)
 - .lzma/.xz (Lempel-Ziv-Markov chain-Algorithm format)
 - .a, .lib (ar tool format, COFF/a.out/b.out) / extract only
 - .rpm (RPM package) (=leading + signature + header + .cpio.gz) /extract only
 - .deb (Debian Package) (=ar of ("debian-binary" + "control.tar.gz" + "data.tar.gz")) /extract only
 - .tar.gz(.tgz), .tar.bz2, .tar.lzma, .tar.xz, .tar.zst
 - .tar.Z(.taz) / extract only
 - .cpio.gz, .cpio.Z, .cpio.bz2 /extract only
 - .a.gz, .a.Z, .a.bz2, lib.gz, lib.Z, lib.bz2, lib.gz / extract only


## 機能
.tar/.tar.gz/.tar.Z/.gz/.Z/.bz2/.lzma/.xz/.zst形式の圧縮/展開が行えます。ただし、.tar.Z/.Zについては展開のみ行えます。

統合アーカイバプロジェクトのAPI仕様に準拠。TAR32.DLLのAPIに対応した多数のアプリケーションで利用できます。

## 動作環境
Windows 10以降で動作を確認しています。それ以前のOSでも動作するかもしれませんが、確認はしておりません。

## 使い方
### アプリケーション利用者

- エクスプローラの設定によってはTAR32.DLLが見えない場合があります。その場合はエクスプローラのメニューバーの表示/オプションでオプションダイアログを出し、「全てのファイルを表示」をチェックし、「登録されているファイルの拡張子は表示しない」のチェックを外してください。
 - TAR32.DLLをWindowsのSYSTEMフォルダにコピーしてください。
   - 64bit OSではC:\Windows\SysWOW64となります。
 - TAR32.DLLに対応したアプリケーションプログラムをお使いください。

### アプリケーション開発者

 - 基本的なAPIの使い方はUNLHA32.DLLとほぼ同じです。UNLHA32.DLLのドキュメントを参考にしてください。

# 開発について

このレポジトリは http://openlab.ring.gr.jp/tsuneo/tar32/ から派生したものです。gitのsubmodule機能を使用してライブラリ更新を容易にするため、github上にforkさせました。

# License
Public Domain
