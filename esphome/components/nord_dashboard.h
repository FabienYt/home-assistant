#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <string>
#include <vector>

namespace nord {

static const size_t HOURLY_MAX = 12;
// hour, condition, temperature, rain, wind
using HourlyEntry = std::array<std::string, 5>;

// Parses "13h|rainy|14°|0.1 mm|36 km/h;14h|...". Malformed entries are skipped.
inline std::vector<HourlyEntry> parse_hourly(const std::string &packed) {
  std::vector<HourlyEntry> out;
  size_t start = 0;
  while (start < packed.size() && out.size() < HOURLY_MAX) {
    size_t end = packed.find(';', start);
    if (end == std::string::npos) end = packed.size();
    HourlyEntry e;
    size_t field = 0, pos = start;
    while (field < e.size() && pos <= end) {
      size_t sep = packed.find('|', pos);
      if (sep == std::string::npos || sep > end) sep = end;
      e[field++] = packed.substr(pos, sep - pos);
      pos = sep + 1;
    }
    if (field == e.size()) out.push_back(e);
    start = end + 1;
  }
  return out;
}

inline std::string fmt(float v, int decimals) {
  if (std::isnan(v)) return "--";
  char buf[16];
  snprintf(buf, sizeof(buf), "%.*f", decimals, v);
  return buf;
}

// MDI 7.4.47 codepoints
inline const char *weather_glyph(const std::string &c) {
  if (c == "sunny") return "\U000F0599";
  if (c == "clear-night") return "\U000F0594";
  if (c == "partlycloudy") return "\U000F0595";
  if (c == "cloudy") return "\U000F0590";
  if (c == "fog") return "\U000F0591";
  if (c == "hail") return "\U000F0592";
  if (c == "lightning") return "\U000F0593";
  if (c == "lightning-rainy") return "\U000F067E";
  if (c == "pouring") return "\U000F0596";
  if (c == "rainy") return "\U000F0597";
  if (c == "snowy") return "\U000F0598";
  if (c == "snowy-rainy") return "\U000F067F";
  if (c == "windy") return "\U000F059D";
  if (c == "windy-variant") return "\U000F059E";
  if (c == "exceptional") return "\U000F05D6";
  return "\U000F02D6";
}

inline const char *weather_label(const std::string &c) {
  if (c == "sunny") return "Ensoleillé";
  if (c == "clear-night") return "Nuit claire";
  if (c == "partlycloudy") return "Éclaircies";
  if (c == "cloudy") return "Nuageux";
  if (c == "fog") return "Brouillard";
  if (c == "hail") return "Grêle";
  if (c == "lightning") return "Orages";
  if (c == "lightning-rainy") return "Orages et pluie";
  if (c == "pouring") return "Averses fortes";
  if (c == "rainy") return "Pluie";
  if (c == "snowy") return "Neige";
  if (c == "snowy-rainy") return "Pluie et neige";
  if (c == "windy" || c == "windy-variant") return "Venteux";
  if (c == "exceptional") return "Exceptionnel";
  return "Indisponible";
}

inline const char *wind_dir(float deg) {
  static const char *const DIRS[] = {"N", "NE", "E", "SE", "S", "SO", "O", "NO"};
  if (std::isnan(deg)) return "";
  int i = static_cast<int>(std::lround(deg / 45.0f)) % 8;
  return DIRS[i < 0 ? i + 8 : i];
}

inline const char *hvac_label(const std::string &s) {
  if (s == "heat") return "Chauffe";
  if (s == "cool") return "Froid";
  if (s == "heat_cool") return "Auto";
  if (s == "dry") return "Déshumidification";
  if (s == "fan_only") return "Ventilation";
  if (s == "off") return "Arrêt";
  return "Indisponible";
}

// Matter air quality states. Level: 0 good, 1 moderate, 2 poor, 3 very poor, -1 unknown.
inline int air_level(const std::string &s) {
  if (s == "good" || s == "fair") return 0;
  if (s == "moderate") return 1;
  if (s == "poor") return 2;
  if (s == "very_poor" || s == "extremely_poor") return 3;
  return -1;
}

inline const char *air_label(const std::string &s) {
  if (s == "good") return "Bonne";
  if (s == "fair") return "Correcte";
  if (s == "moderate") return "Moyenne";
  if (s == "poor") return "Mauvaise";
  if (s == "very_poor") return "Très mauvaise";
  if (s == "extremely_poor") return "Extrêmement mauvaise";
  return "Inconnue";
}

// nord14 / nord13 / nord12 / nord11; nord12 and nord11 are only used for large text or graphics.
inline uint32_t level_color(int level) {
  switch (level) {
    case 0: return 0xA3BE8C;
    case 1: return 0xEBCB8B;
    case 2: return 0xD08770;
    case 3: return 0xBF616A;
    default: return 0xD8DEE9;
  }
}

inline int threshold_level(float v, float warn, float bad) {
  if (std::isnan(v)) return -1;
  if (v >= bad) return 2;
  if (v >= warn) return 1;
  return 0;
}

// Beaufort-inspired: < 20 km/h calm (nord4), 20-39 moderate (nord13), >= 40 strong (nord12, large text only).
inline uint32_t wind_color(int kmh) {
  if (kmh >= 40) return 0xD08770;
  if (kmh >= 20) return 0xEBCB8B;
  return 0xD8DEE9;
}

// ESPTime: day_of_week 1 = Sunday, month 1 = January.
inline std::string french_date(int day_of_week, int day_of_month, int month) {
  static const char *const DAYS[] = {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"};
  static const char *const MONTHS[] = {"janvier", "février", "mars", "avril", "mai", "juin",
                                       "juillet", "août", "septembre", "octobre", "novembre", "décembre"};
  if (day_of_week < 1 || day_of_week > 7 || month < 1 || month > 12) return "";
  return std::string(DAYS[day_of_week - 1]) + " " + std::to_string(day_of_month) + " " + MONTHS[month - 1];
}

// Log scale between lux_min and lux_max, since perceived brightness is logarithmic.
inline float backlight_level(float lux, float lux_min, float lux_max, float level_min) {
  if (std::isnan(lux)) return 0.5f;
  if (lux <= lux_min) return level_min;
  if (lux >= lux_max) return 1.0f;
  float t = (std::log10(lux) - std::log10(lux_min)) / (std::log10(lux_max) - std::log10(lux_min));
  return level_min + (1.0f - level_min) * t;
}

// Minutes since midnight; the window may wrap past midnight.
inline bool in_window(int now, int start, int end) {
  if (start == end) return false;
  if (start < end) return now >= start && now < end;
  return now >= start || now < end;
}

#ifdef USE_LVGL
inline lv_obj_t *hourly_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color, int32_t width) {
  lv_obj_t *label = lv_label_create(parent);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
  lv_obj_set_width(label, width);
  lv_label_set_text(label, "");
  return label;
}

// Rows are built on first call inside the YAML grid object, then only their texts change.
// Fixed label widths keep the values aligned from one row to the next.
inline void hourly_update(lv_obj_t *grid, const std::string &packed, const lv_font_t *body,
                          const lv_font_t *value, const lv_font_t *icon) {
  static lv_obj_t *rows[HOURLY_MAX] = {};
  static lv_obj_t *cells[HOURLY_MAX][5] = {};
  if (rows[0] == nullptr) {
    // ESPHome only accepts a flex layout on objects with YAML children, so it is set here.
    // COLUMN_WRAP fills the left column first: 6 rows per column with the grid height from YAML.
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_set_style_pad_row(grid, 10, 0);
    lv_obj_set_style_pad_column(grid, 16, 0);
    for (size_t i = 0; i < HOURLY_MAX; i++) {
      lv_obj_t *row = lv_obj_create(grid);
      lv_obj_remove_style_all(row);
      lv_obj_remove_flag(row, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_size(row, 480, 68);
      lv_obj_set_style_bg_color(row, lv_color_hex(0x3B4252), 0);
      lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
      lv_obj_set_style_radius(row, 12, 0);
      lv_obj_set_style_pad_left(row, 20, 0);
      lv_obj_set_style_pad_right(row, 20, 0);
      lv_obj_set_style_pad_column(row, 8, 0);
      lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
      lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
      // Rain and wind use the 28px font: large text, so nord12 for strong wind keeps >= 3:1.
      cells[i][0] = hourly_label(row, value, 0x88C0D0, 56);
      cells[i][1] = hourly_label(row, icon, 0x88C0D0, 44);
      cells[i][2] = hourly_label(row, value, 0xECEFF4, 64);
      cells[i][3] = hourly_label(row, value, 0x88C0D0, 120);
      cells[i][4] = hourly_label(row, value, 0xD8DEE9, 120);
      rows[i] = row;
    }
  }
  auto entries = parse_hourly(packed);
  for (size_t i = 0; i < HOURLY_MAX; i++) {
    if (i >= entries.size()) {
      lv_obj_add_flag(rows[i], LV_OBJ_FLAG_HIDDEN);
      continue;
    }
    lv_obj_remove_flag(rows[i], LV_OBJ_FLAG_HIDDEN);
    const auto &e = entries[i];
    bool dry = e[3].rfind("0.0", 0) == 0;
    lv_label_set_text(cells[i][0], e[0].c_str());
    lv_label_set_text(cells[i][1], weather_glyph(e[1]));
    lv_label_set_text(cells[i][2], e[2].c_str());
    lv_label_set_text(cells[i][3], dry ? "" : e[3].c_str());
    lv_label_set_text(cells[i][4], e[4].c_str());
    lv_obj_set_style_text_color(cells[i][4], lv_color_hex(wind_color(std::atoi(e[4].c_str()))), 0);
  }
}
#endif

}  // namespace nord
