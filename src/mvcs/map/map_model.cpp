#include "map_model.h"

#include <fstream>

#include <QMessageBox>

static inline bool checkExtension(const std::string& str, const std::string& extension) {
    if (str.length() < extension.length() + 1) {
        return false;
    }

    size_t ext_start_index = str.find_last_of('.');
    if (ext_start_index == std::string::npos ||
        str.length() - (ext_start_index + 1) != extension.length() ||
        str.substr(ext_start_index + 1) != extension
    ) {
        return false;
    }

    return true;
}

Map_Model::Map_Model(const std::string& cubmap_filepath) {
    readMap(cubmap_filepath);
}

void Map_Model::newMap(u32 width, u32 height) {
    if (width < 3 || height < 3) {
        throw std::runtime_error("can't have a map with less than 3 dimensions");
    }

    emit layoutAboutToBeChanged();

    cells.clear();
    cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width, Cell::Empty));
    camera = Camera(
        (r32) width / 2.0f + 0.5f,
        (r32) height / 2.0f + 0.5f,
        camera.phi
    );
    for (u32 row = 1; row < cells.size() - 1; ++row) {
        cells[row].back() = Cell::Wall;
        cells[row].front() = Cell::Wall;
    }
    for (u32 col = 0; col < cells[0].size(); ++col) {
        cells.back()[col] = Cell::Wall;
        cells.front()[col] = Cell::Wall;
    }
    cells[(i32)camera.p.y][(i32)camera.p.x] = Cell::Player;

    emit layoutChanged();
}

void Map_Model::readMap(const std::string& cubmap_filepath) {
    if (checkExtension(cubmap_filepath, "cub") == false) {
        throw std::runtime_error("file doesn't have .cub extension");
    }

    std::ifstream ifs(cubmap_filepath);
    if (!ifs) {
        throw std::runtime_error("couldnt open file path for cubmap: '" + cubmap_filepath + "'");
    }

    std::array<LoadedImageFromFile, static_cast<u32>(WallTexId::SIZE)> cur_wall_textures;
    LoadedImageFromFile cur_floor_tex;
    LoadedImageFromFile cur_ceiling_tex;
    try {
        loadTextures(ifs, cur_wall_textures, cur_floor_tex, cur_ceiling_tex);
    } catch (const std::exception& err) {
        throw std::runtime_error("'loadTextures': " + std::string(err.what()));
    }

    std::string line;
    if (!std::getline(ifs, line) || line != "") {
        throw std::runtime_error("there must follow at least one newline after texture declarations");
    }

    std::vector<std::vector<Cell>> cur_cells;
    Camera cur_camera;
    bool found_player = false;
    u32 max_row_size = 0;
    bool skipped_newlines = false;
    while (std::getline(ifs, line)) {
        if (skipped_newlines == false) {
            if (line != "") {
                skipped_newlines = true;
            } else {
                continue ;
            }
        } else if (line == "") {
            break ;
        }

        std::vector<Cell> cur_row;
        if (line.size() > max_row_size) {
            max_row_size = (u32) line.size();
        }
        for (u32 col = 0; col < line.size(); ++col) {
            char c = line[col];
            r32 orientation;
            Cell cell = charToCell(c, orientation);
            if (cell == Cell::Player) {
                if (found_player) {
                    throw std::runtime_error("found multiple players during parsing the map: " + cubmap_filepath);
                }
                cur_camera = Camera(
                    (r32) col + 0.5f,
                    (r32) cur_cells.size() + 0.5f,
                    orientation
                );
                found_player = true;
            }
            cur_row.push_back(cell);
        }
        cur_cells.push_back(cur_row);
    }
    // note: make map square-like so that each row is the same size for simplicity, this assumption is baked in everywhere
    for (auto& row : cur_cells) {
        if (row.size() < max_row_size) {
            row.insert(row.end(), max_row_size - row.size(), Cell::Wall);
        }
    }
    if (found_player == false) {
        throw std::runtime_error("didn't find player during parsing the map: " + cubmap_filepath);
    }

    if (isMapEnclosed(cur_cells) == false) {
        throw std::runtime_error("map is not enclosed by walls: " + cubmap_filepath);
    }

    // note: replace the old data with the new after successful parsing and map sanity checking
    assert(sizeof(cur_wall_textures) / sizeof(cur_wall_textures[0]) == sizeof(wall_textures) / sizeof(wall_textures[0]));
    cells = cur_cells;
    camera = cur_camera;
    for (u32 wall_texture_index = 0; wall_texture_index < sizeof(cur_wall_textures) / sizeof(cur_wall_textures[0]); ++wall_texture_index) {
        wall_textures[wall_texture_index] = cur_wall_textures[wall_texture_index];
        // note: baked in assumption for the renderer for now: images are rotated by 90 for sequential access during wall casting
        QTransform wall_transform;
        wall_transform.rotate(-90);
        wall_textures[wall_texture_index] = wall_textures[wall_texture_index].transformed(wall_transform);
    }
    floor_tex = cur_floor_tex;
    ceiling_tex = cur_ceiling_tex;
}

void Map_Model::saveMap(const std::string& cubmap_filepath) {
    std::ofstream savedFile(cubmap_filepath);
    if (!savedFile) {
        throw std::runtime_error("Unable to open file for writing");
    }
    savedFile << "NO ";
    savedFile << wall_textures[static_cast<u32>(WallTexId::North)].getFilePath();
    savedFile << std::endl;
    savedFile << "SO ";
    savedFile << wall_textures[static_cast<u32>(WallTexId::South)].getFilePath();
    savedFile << std::endl;
    savedFile << "WE ";
    savedFile << wall_textures[static_cast<u32>(WallTexId::West)].getFilePath();
    savedFile << std::endl;
    savedFile << "EA ";
    savedFile << wall_textures[static_cast<u32>(WallTexId::East)].getFilePath();
    savedFile << std::endl;
    savedFile << "F ";
    savedFile << floor_tex.getFilePath();
    savedFile << std::endl;
    savedFile << "C ";
    savedFile << ceiling_tex.getFilePath();
    savedFile << std::endl;

    savedFile << std::endl;
    for (u32 row = 0; row < rowCount(); ++row) {
        for (u32 col = 0; col < colCount(); ++col) {
            savedFile << cellToChar(getData(col, row), camera.phi);
        }
        savedFile << std::endl;
    }
}

int Map_Model::rowCount(const QModelIndex &parent) const {
    return static_cast<i32>(cells.size());
}

int Map_Model::columnCount(const QModelIndex &parent) const {
    if (rowCount(parent) == 0) {
        return 0;
    }
    return static_cast<i32>(cells[0].size());
}

Map_Model::Cell Map_Model::charToCell(char c, r32& orientation) const {
    switch (c) {
        case '0': {
            return Cell::Empty;
        } break ;
        case '1': {
            return Cell::Wall;
        } break ;
        case 'N': {
            r32 pi = std::atan(1.0f) * 4.0f;
            orientation = -pi / 2.0f;
            return Cell::Player;
        } break ;
        case 'S': {
            r32 pi = std::atan(1.0f) * 4.0f;
            orientation = pi / 2.0f;
            return Cell::Player;
        } break ;
        case 'W': {
            r32 pi = std::atan(1.0f) * 4.0f;
            orientation = pi;
            return Cell::Player;
        } break ;
        case 'E': {
            orientation = 0.0f;
            return Cell::Player;
        } break ;
        case ' ': {
            return Cell::Wall;
        } break ;
        default: {
            throw std::runtime_error("unexpected character");
        }
    }

    return Cell::Empty;
}

char Map_Model::cellToChar(Cell cell, r32 orientation) const {
    switch (cell) {
        case Cell::Empty: {
            return '0';
        } break ;
        case Cell::Wall: {
            return '1';
        } break ;
        case Cell::Player: {
            r32 cosRes = cos(orientation);
            r32 sinRes = sin(orientation);
            char vertical_orientation;
            char horizontal_orientation;
            if (cosRes > 0.0f) {
                vertical_orientation = 'E';
            } else {
                vertical_orientation = 'W';
            }
            if (sinRes > 0.0f) {
                horizontal_orientation = 'S';
            } else {
                horizontal_orientation = 'N';
            }
            if (std::fabs(cosRes) > std::fabs(sinRes)) {
                return vertical_orientation;
            } else {
                return horizontal_orientation;
            }
        } break ;
        default: {
            throw std::runtime_error("unexpected cell value");
        }
    }

    return '\0';
}

QVariant Map_Model::data(const QModelIndex &index, int role) const {
    if (index.isValid() == false) {
        return QVariant();
    }

    if (isIndexValid(index.column(), index.row()) == false) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return static_cast<i32>(cells[index.row()][index.column()]);
    } else {
        return QVariant();
    }
}

QVariant Map_Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QStringLiteral("Column %1").arg(section);
    } else {
        return QStringLiteral("Row %1").arg(section);
    }
}

Qt::ItemFlags Map_Model::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool Map_Model::insertRows(int row, int count, const QModelIndex &parent) {
    assert(row >= 0 && count >= 0);

    if (row >= rowCount(parent)) {
        row = rowCount(parent) - 1;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);
    cells.insert(cells.begin() + row, count, std::vector<Cell>(columnCount(parent), Cell::Wall));
    endInsertRows();

    return true;
}

bool Map_Model::insertColumns(int column, int count, const QModelIndex &parent) {
    assert(column >= 0 && count >= 0);

    if (column > columnCount(parent)) {
        column = columnCount(parent);
    }
    beginInsertColumns(QModelIndex(), column, column + count - 1);
    for (auto& row: cells) {
        row.insert(row.begin() + column, count, Cell::Wall);
    }
    endInsertColumns();

    return true;
}

bool Map_Model::removeRows(int row, int count, const QModelIndex &parent) {
    assert(row >= 0 && count >= 0);

    if (rowCount(parent) == 0) {
        return false;
    }

    if (row >= rowCount(parent)) {
        row = rowCount(parent) - 1;
    }
    if (row + count >= rowCount(parent)) {
        count = rowCount(parent) - 1 - row;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    cells.erase(cells.begin() + row, cells.begin() + row + count);
    endRemoveRows();

    return true;
}

bool Map_Model::removeColumns(int column, int count, const QModelIndex &parent) {
    assert(column >= 0 && count >= 0);

    if (columnCount(parent) == 0) {
        return false;
    }

    if (column >= columnCount(parent)) {
        column = columnCount(parent) - 1;
    }
    if (column + count >= columnCount(parent)) {
        count = columnCount(parent) - 1 - column;
    }

    beginRemoveColumns(QModelIndex(), column, column + count - 1);
    for (auto& row: cells) {
        row.erase(row.begin() + column, row.begin() + column + count);
    }
    endRemoveColumns();

    return true;
}

Map_Model::Cell Map_Model::getData(u32 col, u32 row) const {
    return cells[row][col];
}

u32 Map_Model::rowCount() const {
    return static_cast<u32>(cells.size());
}

u32 Map_Model::colCount() const {
    if (rowCount() == 0) {
        return 0;
    }

    return static_cast<u32>(cells[0].size());
}

bool Map_Model::isIndexValid(u32 col, u32 row) const {
    return row < cells.size() && col < cells[row].size();
}

bool Map_Model::isPWalkable(u32 col, u32 row) {
    if (isIndexValid(col, row) &&
        cells[row][col] == Map_Model::Cell::Empty ||
        cells[row][col] == Map_Model::Cell::Player
    ) {
        return true;
    }

    return false;
}

bool Map_Model::setData(u32 col, u32 row, Map_Model::Cell value) {
    return setData(createIndex(row, col), QVariant(static_cast<i32>(value)), Qt::EditRole);
}

bool Map_Model::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() == false) {
        return false;
    }

    if (isIndexValid(index.column(), index.row()) == false) {
        return false;
    }

    bool conversion_was_successful;
    Map_Model::Cell cell_value = static_cast<Map_Model::Cell>(value.toInt(&conversion_was_successful));
    if (conversion_was_successful == false) {
        return false;
    }

    Map_Model::Cell old_cell_value = cells[index.row()][index.column()];
    if (role == Qt::EditRole && old_cell_value != cell_value) {
        cells[index.row()][index.column()] = cell_value;
        if (isMapEnclosed(cells) == false) {
            cells[index.row()][index.column()] = old_cell_value;

            return false;
        } else {
            emit dataChanged(index, index, {role});

            return true;
        }
    }

    return true;
}

bool Map_Model::isMapEnclosed(const std::vector<std::vector<Cell>>& m) const {
    if (m.size() == 0 || m[0].size() == 0) {
        return false;
    }

    for (u32 col = 0; col < m[0].size(); ++col) {
        if (m.back()[col] != Cell::Wall ||
            m.front()[col] != Cell::Wall
        ) {
            return false;
        }
    }
    for (u32 row = 1; row < m.size() - 1; ++row) {
        if (m[row].front() != Cell::Wall ||
            m[row].back() != Cell::Wall
        ) {
            return false;
        }
    }

    return true;
}

void Map_Model::loadTextures(
    std::ifstream& ifs,
    std::array<LoadedImageFromFile, static_cast<u32>(WallTexId::SIZE)>& cur_wall_textures,
    LoadedImageFromFile& cur_floor_tex,
    LoadedImageFromFile& cur_ceiling_tex
) {
    std::string line;
    u32 num_of_texture_files_to_parse = 0;
    num_of_texture_files_to_parse += sizeof(cur_wall_textures) / sizeof(cur_wall_textures[0]); // walls
    ++num_of_texture_files_to_parse; // floor
    ++num_of_texture_files_to_parse; // ceil
    while (num_of_texture_files_to_parse > 0) {
        if (!std::getline(ifs, line)) {
            throw std::runtime_error("unexpected end of file during texture loading");
        }
        std::string::size_type spaceIndex = line.find_first_of(' ');
        if (spaceIndex == std::string::npos) {
            throw std::runtime_error("space character not found after texture-id");
        }
        std::string textureId(line.begin(), line.begin() + spaceIndex);
        std::string texturePath(line.begin() + spaceIndex + 1, line.end());
        if (textureId == "NO") {
            if (cur_wall_textures[static_cast<u32>(WallTexId::North)].isNull() == false) {
                throw std::runtime_error("duplicate north wall texture");
            }
            cur_wall_textures[static_cast<u32>(WallTexId::North)] = LoadedImageFromFile(texturePath);
            if (cur_wall_textures[static_cast<u32>(WallTexId::North)].isNull()) {
                throw std::runtime_error("failed to parse north wall texture from path: " + texturePath);
            }
            assert(num_of_texture_files_to_parse > 0);
            --num_of_texture_files_to_parse;
        } else if (textureId == "SO") {
            if (cur_wall_textures[static_cast<u32>(WallTexId::South)].isNull() == false) {
                throw std::runtime_error("duplicate south wall texture");
            }
            cur_wall_textures[static_cast<u32>(WallTexId::South)] = LoadedImageFromFile(texturePath);
            if (cur_wall_textures[static_cast<u32>(WallTexId::South)].isNull()) {
                throw std::runtime_error("failed to open south wall texture from path: " + texturePath);
            }
            assert(num_of_texture_files_to_parse > 0);
            --num_of_texture_files_to_parse;
        } else if (textureId == "EA") {
            if (cur_wall_textures[static_cast<u32>(WallTexId::East)].isNull() == false) {
                throw std::runtime_error("duplicate east wall texture");
            }
            cur_wall_textures[static_cast<u32>(WallTexId::East)] = LoadedImageFromFile(texturePath);
            if (cur_wall_textures[static_cast<u32>(WallTexId::East)].isNull()) {
                throw std::runtime_error("failed to open east wall texture from path: " + texturePath);
            }
            assert(num_of_texture_files_to_parse > 0);
            --num_of_texture_files_to_parse;
        } else if (textureId == "WE") {
            if (cur_wall_textures[static_cast<u32>(WallTexId::West)].isNull() == false) {
                throw std::runtime_error("duplicate west wall texture");
            }
            cur_wall_textures[static_cast<u32>(WallTexId::West)] = LoadedImageFromFile(texturePath);
            if (cur_wall_textures[static_cast<u32>(WallTexId::West)].isNull()) {
                throw std::runtime_error("failed to open west wall texture from path: " + texturePath);
            }
            assert(num_of_texture_files_to_parse > 0);
            --num_of_texture_files_to_parse;
        } else if (textureId == "F") {
            if (cur_floor_tex.isNull() == false) {
                throw std::runtime_error("duplicate floor texture");
            }
            cur_floor_tex = LoadedImageFromFile(texturePath);
            if (cur_floor_tex.isNull()) {
                throw std::runtime_error("failed to open floor texture from path: " + texturePath);
            }
            assert(num_of_texture_files_to_parse > 0);
            --num_of_texture_files_to_parse;
        } else if (textureId == "C") {
            if (cur_ceiling_tex.isNull() == false) {
                throw std::runtime_error("duplicate ceiling texture");
            }
            cur_ceiling_tex = LoadedImageFromFile(texturePath);
            if (cur_ceiling_tex.isNull()) {
                throw std::runtime_error("failed to open ceiling texture from path: " + texturePath);
            }
            assert(num_of_texture_files_to_parse > 0);
            --num_of_texture_files_to_parse;
        } else {
            throw std::runtime_error("unexpected texture-id: " + textureId);
        }
    }
}
