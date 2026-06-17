package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"image"
	_ "image/jpeg"
	_ "image/png"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/bogem/id3v2/v2"
	"golang.org/x/image/bmp"
	"golang.org/x/image/draw"
)

func main() {
	if len(os.Args) < 3 {
		fmt.Println("Usage: song_org <original_directory> <destination_directory>")
		os.Exit(1)
	}
	originalDir := os.Args[1]
	destinationDir := os.Args[2]

	err := filepath.Walk(originalDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if info.IsDir() || filepath.Ext(path) != ".mp3" {
			return nil
		}
		return processTrack(path, destinationDir)
	})
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		os.Exit(1)
	}
}

func processTrack(path, destinationDir string) error {
	tag, err := id3v2.Open(path, id3v2.Options{Parse: true})
	if err != nil {
		return fmt.Errorf("open tag %s: %w", path, err)
	}
	defer tag.Close()

	artist := orDefault(tag.Artist(), "unknown artist")
	album := orDefault(tag.Album(), "unknown album")
	title := orDefault(tag.Title(), strings.TrimSuffix(filepath.Base(path), ".mp3"))

	trackStr := tag.GetTextFrame(tag.CommonID("Track number/Position in set")).Text
	trackStr = strings.TrimSpace(strings.Split(trackStr, "/")[0])
	trackNum, err := strconv.Atoi(trackStr)
	if err != nil {
		// set to 0 if track number is not available or invalid
		trackNum = 0
	}

	destDir := filepath.Join(destinationDir, artist, album)
	if err = os.MkdirAll(destDir, os.ModePerm); err != nil {
		return fmt.Errorf("mkdir %s: %w", destDir, err)
	}

	// copy mp3
	destFilePath := filepath.Join(destDir, fmt.Sprintf("%02d - %s.mp3", trackNum, title))

	if _, err := os.Stat(destFilePath); os.IsNotExist(err) {
		if err := copyFile(path, destFilePath); err != nil {
			return fmt.Errorf("copy mp3: %w", err)
		}
	} else {
		fmt.Printf("WARN: file already exists, skipping copy: %s\n", destFilePath)
	}

	// cover art

	// check if cover doesn't alr exist

	var color string

	cover300Path := filepath.Join(destDir, "cover_300.bmp")
	_, err = os.Stat(cover300Path)

	if err == nil {
		fmt.Printf("INFO: cover_300.bmp already exists for %s, skipping cover art extraction\n", path)
		// extract color from existing cover_300.bmp
		coverFile, err := os.Open(cover300Path)
		if err != nil {
			return fmt.Errorf("open existing cover_300.bmp: %w", err)
		}
		defer coverFile.Close()

		img, _, err := image.Decode(coverFile)
		if err != nil {
			return fmt.Errorf("decode existing cover_300.bmp: %w", err)
		}

		r, g, b := averageColor(img)
		color = fmt.Sprintf("%02X%02X%02X", r, g, b)
	} else {

		color = "000000"
		img, ok := extractCover(tag)
		if !ok {
			fmt.Printf("INFO: no embedded art for %s, trying MusicBrainz...\n", path)
			img, ok = fetchCover(artist, album)
		}
		if ok {
			if err := writeBMP(img, filepath.Join(destDir, "cover_300.bmp"), 300, 300); err != nil {
				fmt.Printf("WARN: cover_300 failed for %s: %v\n", path, err)
			}
			if err := writeBMP(img, filepath.Join(destDir, "cover_80.bmp"), 80, 80); err != nil {
				fmt.Printf("WARN: cover_80 failed for %s: %v\n", path, err)
			}
			r, g, b := averageColor(img)
			color = fmt.Sprintf("%02X%02X%02X", r, g, b)
		} else {
			fmt.Printf("WARN: no cover art found for %s, using black\n", path)
			black := image.NewRGBA(image.Rect(0, 0, 1, 1))
			writeBMP(black, filepath.Join(destDir, "cover_300.bmp"), 300, 300)
			writeBMP(black, filepath.Join(destDir, "cover_80.bmp"), 80, 80)
			color = "404040" // dark gray so it's not pure nothing
		}
	}

	// write txt
	txtPath := filepath.Join(destDir, fmt.Sprintf("%02d - %s.txt", trackNum, title))
	if err := writeTxt(txtPath, title, artist, album, trackNum, color); err != nil {
		return fmt.Errorf("write txt: %w", err)
	}

	return nil
}

func extractCover(tag *id3v2.Tag) (image.Image, bool) {
	frames := tag.GetFrames(tag.CommonID("Attached picture"))
	if len(frames) == 0 {
		return nil, false
	}
	pic, ok := frames[0].(id3v2.PictureFrame)
	if !ok {
		return nil, false
	}
	img, _, err := image.Decode(bytes.NewReader(pic.Picture))
	if err != nil {
		return nil, false
	}
	return img, true
}

func writeBMP(src image.Image, path string, w, h int) error {
	dst := image.NewRGBA(image.Rect(0, 0, w, h))
	draw.BiLinear.Scale(dst, dst.Bounds(), src, src.Bounds(), draw.Over, nil)
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()
	return bmp.Encode(f, dst)
}

func writeTxt(path, title, artist, album string, track int, color string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()
	fmt.Fprintf(f, "title=%s\n", title)
	fmt.Fprintf(f, "artist=%s\n", artist)
	fmt.Fprintf(f, "album=%s\n", album)
	fmt.Fprintf(f, "track=%d\n", track)
	fmt.Fprintf(f, "color=%s\n", color)
	return nil
}

func averageColor(img image.Image) (uint8, uint8, uint8) {
	bounds := img.Bounds()
	var r, g, b uint64
	count := uint64(bounds.Dx() * bounds.Dy())
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			pr, pg, pb, _ := img.At(x, y).RGBA()
			r += uint64(pr >> 8)
			g += uint64(pg >> 8)
			b += uint64(pb >> 8)
		}
	}
	return uint8(r / count), uint8(g / count), uint8(b / count)
}

func orDefault(s, def string) string {
	if s == "" {
		return def
	}
	return s
}

func copyFile(src, dst string) error {
	in, err := os.Open(src)
	if err != nil {
		return err
	}
	defer in.Close()

	out, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer out.Close()

	_, err = io.Copy(out, in)
	if err != nil {
		return err
	}
	return out.Sync()
}

func fetchCover(artist, album string) (image.Image, bool) {
	query := url.QueryEscape(fmt.Sprintf(`release:"%s" AND artist:"%s"`, album, artist))
	mbURL := fmt.Sprintf("https://musicbrainz.org/ws/2/release/?query=%s&limit=1&fmt=json", query)

	req, _ := http.NewRequest("GET", mbURL, nil)
	req.Header.Set("User-Agent", "song_org/1.0 (your@email.com)")
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, false
	}
	defer resp.Body.Close()

	var result struct {
		Releases []struct {
			ID string `json:"id"`
		} `json:"releases"`
	}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil || len(result.Releases) == 0 {
		return nil, false
	}
	releaseID := result.Releases[0].ID

	coverURL := fmt.Sprintf("https://coverartarchive.org/release/%s/front", releaseID)
	coverResp, err := http.Get(coverURL)
	if err != nil || coverResp.StatusCode != 200 {
		return nil, false
	}
	defer coverResp.Body.Close()

	img, _, err := image.Decode(coverResp.Body)
	if err != nil {
		return nil, false
	}
	return img, true
}
