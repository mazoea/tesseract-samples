#include <allheaders.h>
#include <string>

int main()
{
    std::string file_in("sample.jpg");

    // height in pixels of a A4 in 300 dpi page
    static constexpr int A4_like_height = 3800;

    // note - you should use smart pointers to pix!
    // note  - SEL/Sel PIX/Pix if preferred
    Pix* pixs = pixRead(file_in.c_str());

    // scale to 300dpi
    // note - or you could guess letter size
    Pix* pix_scaled = nullptr;
    {
        float really_scaled = 0.f;
        float scale_to_a4 = 0.f;
        auto bigger_side = std::max(pixGetHeight(pixs), pixGetWidth(pixs));
        l_float32 scale = static_cast<l_float32>(A4_like_height / bigger_side);
        pix_scaled = pixScale(pixs, scale, scale);
    }

    // deskew
    // note - you should guess rotation beforehand
    Pix* pix_deskew = nullptr;
    {
        float angle = {};
        float confidence = {};
        pix_deskew = pixDeskewGeneral(pix_scaled, 0, 40.0, 0.0, 2, 0, &angle, &confidence);
        if (pix_deskew == pix_scaled && 0.0 != angle)
        {
            static constexpr l_float32 DEGREE_TO_RADIAN = 3.14159f / 180.f;
            pix_deskew = pixRotate(pix_scaled, DEGREE_TO_RADIAN * angle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
        }
    }
    
    // note - check if successfull
    Pix* pix_bin = nullptr;
    {
        l_int32 sx = pixGetWidth(pix_deskew);
        l_int32 sy = pixGetHeight(pix_deskew);
        pixOtsuAdaptiveThreshold(pix_deskew, sx, sy, 0, 0, 0.0f, nullptr, &pix_bin);
    }

    // todo - SEL/Sel PIX/Pix if preferred
    Pix* hmt_seeded = nullptr;
    {
        // find bottom fragments of a line (very hardcoded)
        Sel* sel = selCreateFromString(
            "xxxxxxxxxxxxxxxxXxxxxxxxxxxxxxxx"
            "oooooo          x          ooooo"
            "oooooo          x          ooooo"
            "oooooo          x          ooooo",
            4, 32, nullptr);
        Pix* pix_hmt = pixHMT(nullptr, pix_bin, sel);
        selDestroy(&sel);
        hmt_seeded = pixSeedfillBinaryRestricted(nullptr, pix_hmt, pix_bin, 8, 3, 5);
    }

    // note - you should optimise usage of leptonica e.g., use morph string
    Pix* pix_bin_without_lines = nullptr;
    {
        // note - you should compute these dynamically
        static constexpr int letter_w = 20;
        static constexpr int min_line_width = 200;
        // remove narrower (sequential black pixels) objects than 2 * letter_w
        Pix* pix_lines = pixOpenBrick(nullptr, pix_bin, min_line_width, 1);
        pixSeedfillBinaryRestricted(pix_lines, pix_lines, pix_bin, 8, 3, 3);
        pix_bin_without_lines = pixSubtract(nullptr, pix_bin, pix_lines);
    }

    // make letters slightly bigger by filling in the original picture
    pixSeedfillBinaryRestricted(pix_bin_without_lines, pix_bin_without_lines, pix_bin, 8, 3, 2);
    // put back some of the removed fragments - helps OCR
    pixOr(pix_bin_without_lines, pix_bin_without_lines, hmt_seeded);

    pixWrite("processed.png", pix_bin_without_lines, IFF_PNG);

    return 0;
}