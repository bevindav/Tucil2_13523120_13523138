<div align="center">
    <h2>:framed_picture:  Kompresi Gambar dengan Quadtree  :framed_picture:</h2>
    <p>Tugas Kecil (Tucil) 2 IF2211 Strategi Algoritma</p>
    <br/>
</div>

## Deskripsi Program
Quadtree adalah struktur data hierarkis yang digunakan untuk membagi ruang atau data menjadi bagian yang lebih kecil, yang sering digunakan dalam pengolahan gambar. Dalam konteks kompresi gambar, Quadtree membagi gambar menjadi blok-blok kecil berdasarkan keseragaman warna atau intensitas piksel. Struktur ini memungkinkan pengkodean data gambar yang lebih efisien dengan menghilangkan redundansi pada area yang seragam dan mengurangi ukuran file secara signifikan tanpa mengorbankan detail penting pada gambar.

## Requirement Program dan Instalasi Tertentu
Program ini dapat dijalankan dalam terminal lokal(cmd) atau VSCode. Pastikan bahasa C++ sudah diunduh di sistem anda.

## Cara Mengkompilasi & Menjalankan Program
**1.** Buka link repository GitHub, kemudian salin URL di bagian "Code" untuk melakukan git clone

**2.** Buka terminal di VSCode atau CMD

**3.** Arahkan directory ke folder tempat anda ingin melakukan git clone

**4.** Perintahkan git clone
```
git clone https://github.com/bevindav/Tucil2_13523120_13523138.git 
```
**5.** Setelah berhasil, arahkan directory ke folder src
```
cd src
```
**6.** Kompilasikan program program utama dengan perintah berikut:
```
g++ main.cpp quadtree.cpp op.cpp -o main
```
**7.** Setelah berhasil, jalankan program dengan perintah:
```
./main
```
**8.** Ikuti petunjuk yang diberikan untuk memasukkan input dan menyimpan hasil gambar

## Author
| NIM | Nama |
| :---: | :---: |
| 13523120 | Bevinda Vivian |
| 13523138 | Samantha Laqueenna Ginting |