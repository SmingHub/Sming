JPEGDecoder
===========

JPEG Decoder for Arduino - with added support for sming framework https://github.com/SmingHub/Sming/

Based on [picojpeg](https://github.com/richgel999/picojpeg), which had jpg2tga wrapped into a loader class by MakotoKurauchi and later adapted to the arduino by reaper7.  

概要
----
Arduino 用 JPEG デコーダです。デコーダ部には [picojpeg](https://code.google.com/p/picojpeg/) を使用しています。

サンプルコード
----
###SerialCsvOut

SD カード上の JPEG ファイルをブロックごとにデコードし、シリアルから CSV を出力します。

変更履歴
----
V0.01 - 最初のリリース
