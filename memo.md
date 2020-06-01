<h1>memo</h1>

- コンパイルは Mac 上、アセンブラ実行は docker 上
- C 言語から実行まで<br>
  1. C コンパイル(mac or docker)
  <pre>  $ cc -o tcc tcc.c</pre>
  2. アセンブラへコンパイル(mac or docker)
  <pre>  $ ./tcc 123 > tmp.s</pre>
  3. 機械語へのコンパイル(docker)
  <pre>  $ cc -o tmp tmp.s</pre>
  4. 実行(docker)
  <pre>
  $ ./tmp
  $ echo $?</pre>
- make(docker)<br>
make でコンパイルしたものは docker 上で動かす。
<pre>$ make</pre>
- make test(docker)
<pre>$ make test</pre>

<h1>Docker</h1>
* docker上でコマンド実行
<pre>
$ docker run --rm -v ~/syu_dev/c_compiler/tcc:/tcc -w /tcc compilerbook cc -o tmp tmp.s

command
docker run --rm -v <ホストのディレクトリ>:<コンテナ内ディレクトリ> -w <実行ディレクトリ> <イメージ名> <コマンド>

option
--rm: コマンド終了後にコンテナ削除
-v: ディレクトリのマウント
-w: コマンド実行時のカレントディレクトリ

</pre>

- イメージ一覧
<pre>
$ docker images
</pre>

- コンテナ一覧
<pre>
起動中のコンテナ
$ docker ps

全コンテナ
\$ docker ps -a

</pre>

- コンテナ作成＆起動
<pre>
$ docker run -i -t -d -p 8080:3000 --name comp_book compilerbook

command
docker run -i -t -d -p <ホストのポート>:<コンテナのポート> --name <コンテナ名> <イメージ名>

option
-i: 標準入力の受付(フォアグランドで実行する場合、必須)
-t: 疑似 tty を割り当てる(フォアグランドで実行する場合、必須)
-d: コンテナのデタッチ(バックグラウンド実行)
-p: ホストとコンテナのポートマッピング
--name: コンテナの名前
※オプションは他にも沢山ある。

</pre>

- コンテナの中に入る
<pre>
$ docker attach comp_book

command
docker attach <コンテナ名 or コンテナ ID>
→ コンテナ名、コンテナ ID はコンテナ一覧で確認出来る。

remark
コンテナから抜ける場合は
\$ exit

</pre>

- コンテナの作成
<pre>
$ docker create comp_book

command
docker create <コンテナ名 or コンテナ ID>

</pre>

- コンテナの起動
<pre>
$ docker start comp_book

command
docker start <コンテナ名 or コンテナ ID>

</pre>

- コンテナの停止
<pre>
$ docker stop comp_book

command
docker stop <コンテナ名 or コンテナ ID>

</pre>

- コンテナの削除
<pre>
$ docker rm comp_book

command
docker rm <コンテナ名 or コンテナ ID>

</pre>

- 一括でコンテナの停止、削除
<pre>
一括停止
$ sudo docker kill $(sudo docker ps -a -q)
一括削除
$ sudo docker rm $(sudo docker ps -a -q)
一括停止 & 削除
$ sudo docker ps -a -q | xargs docker kill | xargs docker rm
</pre>

<h1>アセンブラ</h1>

レジスタ種類

- RAX: 整数値を入れるレジスタ。<br>
  関数からの戻り値としても使用される。
- RDI: 第一引数を入れるレジスタ。
- RSI: 第二引数を入れるレジスタ。
- RDX: 第三引数を入れるレジスタ。
- RCX: 第四引数を入れるレジスタ。
- R8: 第五引数を入れるレジスタ。
- R9: 第六引数を入れるレジスタ。
- RBP: ベースポインタとして使用されるレジスタ。
- RSP: スタックポインタとして使用されるレジスタ。
- AL: RAX の下 8bit。

命令

- CALL: 関数呼び出し。
  - call の次の命令のアドレスをスタックにプッシュ。
  - call の引数として与えられたアドレスにジャンプ。
- MOV: データのコピーを行う。<br>
  <pre>
  MOV RAX, RSI
  //  RAX←RSI
  </pre>
- ADD: 加算。<br>
  2 つのレジスタを受け取って、その内容を加算し、結果を第 1 引数のレジスタに書き込む。
  <pre>
  ADD RAX, 20
  // RAX=RAX+20
  </pre>
- SUB: 減算(ADD と同じ考え)。
- IMUL: 乗算(ADD と同じ考え)。
- IDIV: 除算。CQO を実行後に実行<br>
  暗黙のうちに RDX と RAX を取って、それを合わせたものを 128 ビット整数とみなして、<br>
  それを引数のレジスタの 64 ビットの値で割り、商を RAX に、余りを RDX にセットする。
- CQO: RAX に入っている 64 ビットの値を 128 ビットに伸ばして RDX と RAX にセットする。
<pre>
CQO       // {RDX, RAX} = RAX(128bit)
IDIV RDI  // {RAX(商), RDX(余り)} = {RDX, RAX} / RDI
</pre>
- PUSH: スタックトップに要素を積む。<br>
  RSP をスタックポインタとして暗黙的に使用。
- POP: スタックトップから要素を取り出す。<br>
  RSP をスタックポインタとして暗黙的に使用。
- CMP: 比較。<br>
  同一の場合に 1、そうでなければ 0 を フラグレジスタにセット。
- SETE: 直前の CMP 命令で調べた 2 つの値が同じだった場合に、指定されたレジスタに 1 をセットします。<br>
  それ以外の場合は 0 をセット。
- MOVZB: 上位 56 ビットはゼロクリアして、下位 8 ビットに値をセット。
  <pre>
  CMP RAX, RDI  // 比較結果はフラグレジスタへセットされる
  SETE AL       // CMP結果をALへセット(true: 1, false: 0)
  MOVZB RAX, AL // 上位56ビットゼロクリア + AL(8ビット)コピー
  </pre>
