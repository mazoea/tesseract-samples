if [ ! -f ./sample.jpg ]; then
    echo "Downloading..."
    wget "http://www.tdcj.state.tx.us/death_row/dr_info/ruizroland.jpg" -O sample.jpg
fi

echo "Compilation..."
g++ -std=c++11 -I/usr/include/leptonica -o main main.cc -L/usr/lib/ -llept
echo "Executing..."
./main

echo "Tesseract version..."
tesseract -v

echo "Execute this OCR command..."
echo "tesseract -l Latin --psm 3 --oem 1 processed.png stdout"