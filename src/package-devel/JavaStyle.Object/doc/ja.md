JavaStyle.Object-型チェックおよび型変換の構文
====================
このパッケージでは以下の3つの構文が提供されます。
* instanceof
* as
* to
## instanceof
###構文
    (式) instanceof (型名)
型をチェックする。(式)が返す値の型が(型名)と一致していればtrue、それ以外の場合はfalseを返す。

## as
###構文
    (式) as (型名)
ダウンキャストを行う。失敗した場合は実行時エラーとなる。

## to
###構文
    (式) to (型名)
キャストを行う。失敗した場合は実行時エラーとなる。この構文はto(型名)を呼び出す。
    "1" to int // 1
	"1".toint() // 1
