// I USED CURSOR TO VIBECODE THIS

#include <iostream>
#include <complex>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

// Simple and colorful Mandelbrot renderer for the terminal.
// Usage:
//   ./push [width] [height] [max_iter] [--no-color]
// Defaults:
//   width=120, height=40, max_iter=500

namespace {
bool parseInt(const std::string &text, int &outValue) {
    if (text.empty()) return false;
    char *end = nullptr;
    long v = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') return false;
    if (v < 0 || v > 100000) return false; // sanity bounds
    outValue = static_cast<int>(v);
    return true;
}

std::string color256(int colorCode) {
    std::ostringstream oss;
    oss << "\033[38;5;" << colorCode << 'm';
    return oss.str();
}

std::string resetColor() { return "\033[0m"; }

int iterationToColor(int iter, int maxIter) {
    if (iter >= maxIter) return 16; // inside set: near black
    // Smooth coloring: map iteration to 0..1 then to 256-color gradient band
    double t = static_cast<double>(iter) / static_cast<double>(maxIter);
    // Use a few bands in the 256-color space: 21..51 (blue), 51..87 (cyan/green), 88..196 (reds)
    // Blend across ranges to get a nice spectrum
    int band1 = 21;  // deep blue
    int band2 = 51;  // cyan-ish
    int band3 = 87;  // green/yellow
    int band4 = 196; // bright red

    if (t < 0.33) {
        double u = t / 0.33;
        return band1 + static_cast<int>((band2 - band1) * u);
    } else if (t < 0.66) {
        double u = (t - 0.33) / 0.33;
        return band2 + static_cast<int>((band3 - band2) * u);
    } else {
        double u = (t - 0.66) / 0.34;
        if (u > 1.0) u = 1.0;
        return band3 + static_cast<int>((band4 - band3) * u);
    }
}
} // namespace

int main(int argc, char **argv) {
    int width = 120;
    int height = 40;
    int maxIter = 500;
    bool colorEnabled = true;

    // Parse simple positional args and an optional flag
    // ./push [width] [height] [max_iter] [--no-color]
    if (argc >= 2) {
        int v;
        if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
            std::cout << "Mandelbrot (terminal)\n";
            std::cout << "Usage: " << argv[0] << " [width] [height] [max_iter] [--no-color]\n";
            return 0;
        }
        if (parseInt(argv[1], v)) width = v;
    }
    if (argc >= 3) {
        int v;
        if (parseInt(argv[2], v)) height = v;
    }
    if (argc >= 4) {
        int v;
        if (parseInt(argv[3], v)) maxIter = v;
    }
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--no-color") colorEnabled = false;
    }

    // View window of the complex plane
    // Default is a nice overview around the set
    double minRe = -2.5;
    double maxRe = 1.0;
    double minIm = -1.2;
    double maxIm = 1.2;

    // Adjust imaginary range to maintain aspect ratio (characters are tall)
    double aspect = static_cast<double>(height) / static_cast<double>(width);
    double centerIm = (minIm + maxIm) * 0.5;
    double halfIm = ((maxRe - minRe) * 0.5) * aspect;
    minIm = centerIm - halfIm;
    maxIm = centerIm + halfIm;

    // Precompute steps
    double reStep = (maxRe - minRe) / (width - 1);
    double imStep = (maxIm - minIm) / (height - 1);

    // Render
    for (int y = 0; y < height; ++y) {
        double cIm = maxIm - y * imStep;
        for (int x = 0; x < width; ++x) {
            double cRe = minRe + x * reStep;
            std::complex<double> c(cRe, cIm);
            std::complex<double> z(0.0, 0.0);
            int iter = 0;
            // Iterate z = z^2 + c
            while (std::norm(z) <= 4.0 && iter < maxIter) {
                z = z * z + c;
                ++iter;
            }

            if (colorEnabled) {
                int code = iterationToColor(iter, maxIter);
                if (iter >= maxIter) {
                    std::cout << color256(16) << "#"; // inside set
                } else {
                    std::cout << color256(code) << "*"; // outside
                }
                if (x == width - 1) {
                    std::cout << resetColor();
                }
            } else {
                char ch = (iter >= maxIter) ? '#' : " .-+*%@"[iter % 7];
                std::cout << ch;
            }
        }
        std::cout << '\n';
    }

    if (colorEnabled) {
        std::cout << resetColor();
    }

    // Footer hint
    std::cerr << "Rendered Mandelbrot: " << width << "x" << height
              << ", max_iter=" << maxIter
              << (colorEnabled ? ", color=on" : ", color=off")
              << "\n";
    return 0;
}


