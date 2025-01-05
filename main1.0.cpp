#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <ctime>
#include <thread>
#include <deque>
#include <SFML/Audio.hpp> // Simple and Fast Multimedia Library
using namespace sf;
using namespace std;

class sound_effect {
private:
    SoundBuffer read_buffer;
    SoundBuffer write_buffer;
    SoundBuffer check_buffer;
    SoundBuffer sorted_buffer;
    SoundBuffer notsorted_buffer;
    Sound read;
    Sound write;
    Sound check;
    Sound sorted;
    Sound notsorted;
    bool isMuted;
public:
    sound_effect() {
        if (
            read_buffer.loadFromFile("read.wav") &&
            write_buffer.loadFromFile("write.wav") &&
            check_buffer.loadFromFile("check.wav") &&
            sorted_buffer.loadFromFile("hoorah.wav") &&
            notsorted_buffer.loadFromFile("no.wav")) {
            cout << "\nSound loaded perfectly ^^\n";
        }
        read.setBuffer(read_buffer);
        write.setBuffer(write_buffer);
        check.setBuffer(check_buffer);
        sorted.setBuffer(sorted_buffer);
        notsorted.setBuffer(notsorted_buffer);

        isMuted = false;
    }
    void play(int task, float pitch) {
        if (isMuted) return;
        switch (task)
        {
        case 0:
            read.setPitch(pitch);
            read.play();
            break;
        case 1:
            write.setPitch(pitch);
            write.play();
            break;
        case 2:
            check.setPitch(pitch);
            check.play();
            break;
        case 3:
            sorted.play();
            break;
        case 4:
            notsorted.play();
            break;
        default:
            break;
        }
    }
    void stop() {
        read.stop();
        write.stop();
    }
    void toggleMute() {
        stop();
        isMuted ^= true;
    }
};

struct COUNTERS
{
    int c; // Compare counter
    int r; // Read counter
    int w;// Write counter
    int s; // Swap counter
};
class Blocks {
public:
    int amount;
    int max_val;
    int r_delay = 0;
    int w_delay = 1;
    sound_effect& sound;
    COUNTERS counter;
    vector<int> items;

    Blocks(int a, int m, sound_effect& soundobj):sound(soundobj) {
        amount = a; max_val = m;
        reset_counters();

        items.clear();
        for (int i = 1; i <= amount; i++) {
            items.push_back(i * m / a);
        }
    }
    void reset_counters() {
        counter.c = 0;
        counter.r = 0;
        counter.w = 0;
        counter.s = 0;
    }
    void b_swap(int i, int j) {
        counter.s++;
        int t = this->operator[](i); // t = i
        this->operator()(i, this->operator[](j)); // i = j;
        this->operator()(j, t); // j = t;
    }
    float operator[](int i) { //read
        if (i >= amount) {
            cout << "Error: " << i << " is out of bound value" << endl;
            return 0;
        }
        counter.r++;
        float val = items[i];
        sound.play(0, 0.5 + 1 * val / max_val);
        this_thread::sleep_for(chrono::microseconds(r_delay));
        return val;
    }
    void operator()(int i, int val) { //write
        if (i >= amount) {
            cout << "Error: " << i << " is out of bound value" << endl;
            return;
        }
        counter.w++;
        sound.play(0.05, 0.5 + 0.45 * val / max_val);
        this_thread::sleep_for(chrono::microseconds(w_delay));
        items[i] = val;
    }
    int cmp(int i, int j) {
        counter.c++;                                // Update compare counter
        int a = this->operator[](i);                // this used instead of items so as to increase the read counter
        int b = this->operator[](j);
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
    void stopsound() {
        sound.stop();
    }
    COUNTERS getCounters() {
        return counter;
    }
};
class Viewer : public Sprite {
    RenderTexture texture;
    vector<RectangleShape> rects;
    vector<Color> cols;
    int width, height;
    float r_dx; // reference dx

    Blocks& blk;
    void colorizer(int i) {
        rects[i].setFillColor(Color::White);
    }
public:
    Viewer(int w, int h, Blocks& b) :blk(b) {
        width = w; height = h;
        r_dx = float(width / blk.amount);
        rects = vector<RectangleShape>(blk.amount);
        cols = vector<Color>(blk.amount, Color::White);

        texture.create(width, height);
        this->setTexture(texture.getTexture());
    }
    void render() {
        texture.clear();
        for (int i = 0, j = rects.size();i < j;i++) {
            float h = blk.items[i];
            rects[i].setSize(sf::Vector2f(r_dx, h));
            rects[i].setPosition(i * r_dx, height - h);
            rects[i].setFillColor(cols[i]);
            texture.draw(rects[i]);
        }
        texture.display();
    }
    void mark(int i, uint32_t c) {
        if (i >= rects.size()) return;
        cols[i] = sf::Color(c);
    }
    void mark(int s, int e, uint32_t c) {
        fill(cols.begin() + s, cols.begin() + e, Color(c));
    }
    void unmark(int i) {
        if (i >= rects.size()) return;
        cols[i] = Color::White;
    }
    void unmark(int s, int e) {
        fill(cols.begin() + s, cols.begin() + e, Color::White);
    }
    void unmark_all() {
        std::fill(cols.begin(), cols.end(), sf::Color::White);
    }
};
class Algorithms {
private:
    Blocks& data;
    Viewer& view;
    thread sortingThread;
    int algcount;

    void algo() {
        data.reset_counters();
        view.unmark_all();
        switch (selectedAlg) {
        case 0:
            check();
            break;
        case 1:
            shuffle();
            break;
        case 2:
            bubblesort();
            data.stopsound();
            check();
            break;
        case 3:
            selectionsort();
            data.stopsound();
            check();
            break;
        case 4:
            insertionsort();
            data.stopsound();
            check();
            break;
        case 5:
            quicksort();
            data.stopsound();
            check();
            break;
        case 6:
            mergesort();
            data.stopsound();
            check();
            break;
        }
        working = false;
    }
    void check() {
        selectedAlg = 0;
        int n = data.amount - 1, c;
        view.mark(0, 0x00ff00ff);

        for (c = 0;c < n; ++c) {
            float val = data.items[c];
            if (val > data.items[c + 1]) {
                view.mark(c + 1, 0xff0000ff); // Mark red for unsorted elem
                break;
            }
            view.mark(c + 1, 0x00ff00ff);
            this_thread::sleep_for(chrono::milliseconds(2000 / n));
        }
        sorted = (c == n) ? true : false;
        if (sorted) {
            view.mark(n, 0x00ff00ff);//green
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
    void shuffle() {
        random_shuffle(data.items.begin(), data.items.end());
        sorted = false;
    }
    void bubblesort() {
        for (int i = 0, n = data.amount; i != n - 1; ++i) {
            for (int j = 0; j < n - i - 1; ++j) {
                view.mark(j, 0xff0000ff);               // Mark reading index
                if (data.cmp(j, j + 1) == 1)
                    data.b_swap(j, j + 1);
                view.unmark(j);                         // Unmark reading index
            }
        }
    }
    //-----------------------------------------------------------------
    void selectionsort() {
        int min_val_index = 0;
        for (int i = 0, n = data.amount; i < n; ++i) {
            min_val_index = i;

            // Finding the minimum value index
            for (int j = i + 1; j < n; ++j) {
                view.mark(j, 0x411e59ff);                   // Mark min finding index as purple
                if (data.cmp(j, min_val_index) == -1)
                    min_val_index = j;
                view.unmark(j);                             // Unmark min finding index
            }

            if (min_val_index != i) {
                view.mark(i, 0x0000ffff);                   // Mark writing index as red
                data.b_swap(i, min_val_index);
                view.unmark(i);                             // Mark writing index
            }
        }
    }
    //------------------------------------
    void insertionsort() {
        for (int i = 1, k = data.amount; i < k; ++i) {
            view.mark(i, 0xff0000ff);               // Mark reading index as red
            int curr = data[i];
            int j = i - 1;

            while (j >= 0 && data[j] > curr) {
                view.mark(j + 1, 0x0000ffff);         // Mark writing index as blue
                data(j + 1, data[j]);
                view.unmark(j + 1);                   // Unmark writing index
                j--;
            }
            data(j + 1, curr);
            view.unmark(i);                         // Unmark reading index
        }
    }
    //------------------------------------
    void quicksort() {
        q_qs(0, data.amount - 1);
    }
    void q_qs(int low, int high) {
        int _q_pivot_index;
        if (low >= high) return;
        _q_pivot_index = q_fix(low, high);
        view.mark(_q_pivot_index, 0x00ff00ff);              // Mark pivot index as green

        q_qs(low, _q_pivot_index - 1);
        q_qs(_q_pivot_index + 1, high);
        view.unmark(_q_pivot_index);                        // Unmark pivot index
    }
    int q_fix(int low, int high) {
        int i = low;  // Start `i` from `low` instead of `low - 1`

        for (int j = low; j < high; ++j) {
            if (i > 0) view.mark(i - 1, j - 1, 0xde5f5fff);  // Only mark valid indices
            view.mark(j - 1, 0xff0000ff);                   // Mark reading index as red
            if (data.cmp(j, high) == -1) {
                if (i != j) {
                    data.b_swap(i, j);
                }
                i++;
            }
            if (i > 0) view.unmark(i - 1, j);               // Ensure valid unmarking
        }

        data.b_swap(i, high);
        return i;
    }
    //------------------------------------
    void mergesort() {
        m_ms(0, data.amount - 1);
    }
    void m_ms(int left, int right) {
        if (left >= right) return;
        int mid = (left + right) / 2;

        m_ms(left, mid);
        m_ms(mid + 1, right);

        m_merge(left, mid, right);
    }
    void m_merge(int left, int mid, int right) {
        view.mark(left, 0x00ffffff);    // Mark left boundary
        view.mark(right, 0xffff00ff);   // Mark right boundary
         int  left_n = mid - left + 1;
        int  right_n = right - mid;      // right - (mid + 1) + 1

        // Temporary arrays
        int* left_arr = new int[left_n];
        int* right_arr= new int[right_n];

        // Copying arrays
        for (int i = 0; i < left_n; ++i) {
            view.mark(left + i + 1, 0xff0000ff);                // Mark reading index
            left_arr[i] = data[left + i];
            view.unmark(left + i + 1);                          // Unmark reading index
        }
        for (int i = 0; i < right_n; ++i) {
            view.mark(mid + 1 + i, 0xff0000ff);                 // Mark reading index
            right_arr[i] = data[mid + 1 + i];
            view.unmark(mid + 1 + i);                           // Unmark reading index
        }

        // Pointing ints
        int i = 0;
        int j = 0;
        int k = left;

        // Writing to data
        while (i < left_n && j < right_n)
        {
            view.mark(k - 1, 0x0000ffff);           // Mark writing index
            if (left_arr[i] <= right_arr[j]) {
                data(k, left_arr[i]);
                i++;
            }
            else {
                data(k, right_arr[j]);
                j++;
            }
            view.unmark(k - 1);                     // Unmark writing index
            k++;
        }
        while (i < left_n) {
            view.mark(k - 1, 0x0000ffff);           // Mark writing index
            data(k, left_arr[i]);
            i++;
            view.unmark(k - 1);                     // Unmark writing index
            k++;
        }
        while (j < right_n) {
            view.mark(k - 1, 0x0000ffff);           // Mark writing index
            data(k, right_arr[j]);
            j++;
            view.unmark(k - 1);                     // Unmark writing index
            k++;
        }
        view.unmark(left);                      // Unmark left boundary
        view.unmark(right);                     // Unmark right boundary
    }
    //------------------------------------
    const vector<string> alglist = { "Check", "Shuffle", "Bubblesort", "Selection Sort", "Insertion Sort", "Quick Sort", "Merge Sort" };
public:
    bool working;
    bool sorted;
    int selectedAlg;

    Algorithms(Blocks& b, Viewer& v) :data(b), view(v) {
        setalg(0);
        working = false;
        sorted = true;
        algcount = alglist.size();
    }
    void start() {
        if (working) return;
        working = true;
        if (sortingThread.joinable())
            sortingThread.join();
        sortingThread = thread(&Algorithms::algo, this);
    }
    void stop() {
        working = false;
        sortingThread.detach();
    }

    void setalg(int s) {
        if (s == -1) selectedAlg = (selectedAlg + 1) % algcount;
        else if (s == -2)  selectedAlg = (selectedAlg + algcount - 1) % algcount;
        else
            selectedAlg = s;

        if (!working) {
            data.reset_counters();
            view.unmark_all();
        }
    }
    string getalg() {
        return alglist[selectedAlg];
    }
    //--------
    int getAlgIndex() {
        return selectedAlg;
    }

    bool getAlgWorking() {
        return working;
    }
    //--------------
};
//-----iqra code------
class MenuViewer : public Sprite {
    RenderTexture texture;

public:
    MenuViewer(int w, int h) {
        texture.create(w, h);
        this->setTexture(texture.getTexture());
    }
    void displayMenu(int algoIndex, bool algWorking, COUNTERS counter)
    {

        Texture background;
        background.loadFromFile("menu-bg.png");

        Sprite backgroundSprite;
        backgroundSprite.setTexture(background);
        texture.draw(backgroundSprite);

        string algos[7] = { "Check", "Shuffle", "Bubblesort", "Selection Sort", "Insert Sort", "Quick Sort", "Merge Sort" };
        Font font;
        font.loadFromFile("poppins.ttf");

        Text all[7];
        for (int i = 0; i < 7;i++) {
            Text tmp(algos[i], font, 25);
            all[i] = tmp;
        }

        for (int i = 0; i < 7;i++) {
            all[i].setPosition(80, 130 + (61 * i));

            if (i == algoIndex) {
                all[i].setFillColor(Color::Black);
            }
            texture.draw(all[i]);
        }

        if (algWorking) {
            Text status("Working...", font, 16);
            status.setPosition(40, 690);
            texture.draw(status);
        }


        Text heading("Statistics: ", font, 18);
        heading.setPosition(40, 560);
        texture.draw(heading);

        Text cCount("Comparisons: " + to_string(counter.c), font, 16);
        cCount.setPosition(40, 590);
        texture.draw(cCount);

        Text rCount("Reads: " + to_string(counter.r), font, 16);
        rCount.setPosition(40, 610);
        texture.draw(rCount);

        Text wCount("Writes: " + to_string(counter.w), font, 16);
        wCount.setPosition(40, 630);
        texture.draw(wCount);

        Text sCount("Swaps: " + to_string(counter.s), font, 16);
        sCount.setPosition(40, 650);
        texture.draw(sCount);



    }

    void render(int algoIndex, bool algoWorking, COUNTERS counter) {
        texture.clear();
        displayMenu(algoIndex, algoWorking, counter);
        texture.display();
    }
};
//------------
int main() {
    const int FPS = 60;
    const int WIDTH = 1080;
    const int HEIGHT = 720;
    const int BAR_W = 300;
    const int DATA_SIZE = 200;

    RenderWindow win(VideoMode(WIDTH, HEIGHT), "SORT");
    win.setFramerateLimit(FPS);
    Event ev;
    sound_effect soundeff;
    Blocks data(DATA_SIZE, HEIGHT, soundeff);
    Viewer vie(WIDTH-300, HEIGHT, data);
    vie.setPosition(0, 0);
    MenuViewer menuView(300, HEIGHT);
    menuView.setPosition(WIDTH - BAR_W, 0);
    Algorithms alg(data, vie);
    

    while (win.isOpen()) {
        while (win.pollEvent(ev)) {
            // Keybindings
            if (ev.type == sf::Event::KeyPressed) {
                switch (ev.key.code) {
                case sf::Keyboard::Escape:
                    cout << "Closing.../ n";
                    alg.stop();
                    win.close();
                    break;

                case sf::Keyboard::Space:
                    cout << alg.getalg() << "started!/n";
                    alg.start();

                    break;
                case sf::Keyboard::C:
                    alg.setalg(0);
                    break;
                case sf::Keyboard::S:
                    alg.setalg(1);
                    cout << alg.getalg() << "selected!" << endl;
                    break;

                case sf::Keyboard::B:

                    alg.setalg(2);
                    cout << alg.getalg() << "selected!" << endl;
                    break;
                case sf::Keyboard::Up:
                    alg.setalg(-2);
                    cout << alg.getalg() << "selected!" << endl;
                    break;

                case sf::Keyboard::Down:
                    alg.setalg(-1);
                    cout << alg.getalg() << "selected!" << endl;
                    break;
                case Keyboard::M:
                    soundeff.toggleMute();
                    cout <<"Mute Toggled!" << endl;
                    break;

                default:
                    std::cout << "No keybind set for this(" << ev.key.code << ") key!\n";
                }
            }
        }

        vie.render();
        menuView.render(alg.getAlgIndex(), alg.getAlgWorking(), data.getCounters());

        win.clear();
        win.draw(vie);
        win.draw(menuView);
        win.display();

        

    

    }
}