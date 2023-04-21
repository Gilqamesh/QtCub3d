#ifndef MAP_MODEL_H
#define MAP_MODEL_H

#include "../mvcs_defs.h"

#include "../../camera.h"

#include <QAbstractTableModel>
#include <QImage>

#include <vector>

class Map_Model: public QAbstractTableModel {
    Q_OBJECT

public:
    enum class Cell {
        Empty,
        Player,
        Wall
    };

    enum class WallTexId {
        North,
        South,
        West,
        East,

        SIZE
    };

public:
    Map_Model(const std::string& cubmap_filepath);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

signals:
    void cameraCellChanged(const QModelIndex &index);

public:
    // @brief keeps loaded texture files and defines an empty enclosed map with the supplied dimensions
    void newMap(u32 width, u32 height);

    void readMap(const std::string& cubmap_filepath);
    void saveMap(const std::string& cubmap_filepath);

    Cell getData(u32 col, u32 row) const;
    u32 rowCount() const;
    u32 colCount() const;

    bool isIndexValid(u32 col, u32 row) const;
    bool isPWalkable(u32 col, u32 row);

    bool setData(u32 col, u32 row, Map_Model::Cell value);

public: // note: expose these for simplicity for now
    class LoadedImageFromFile: public QImage {
    public:
        LoadedImageFromFile() = default;
        LoadedImageFromFile(const std::string& filepath): QImage(filepath.c_str()), _filepath(filepath) {}
        LoadedImageFromFile& operator=(const QImage& other) {
            QImage::operator=(other);
            return *this;
        }
        std::string getFilePath() const { return _filepath; }
        void setFilePath(const std::string filepath) { _filepath = filepath; }
    private:
        std::string _filepath;
    };
    Camera camera;
    std::array<LoadedImageFromFile, static_cast<u32>(WallTexId::SIZE)> wall_textures;
    LoadedImageFromFile floor_tex;
    LoadedImageFromFile ceiling_tex;

private:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool isMapEnclosed(const std::vector<std::vector<Cell>>& m) const;
    void loadTextures(
        std::ifstream& ifs,
        std::array<LoadedImageFromFile, static_cast<u32>(WallTexId::SIZE)>& cur_wall_textures,
        LoadedImageFromFile& cur_floor_tex,
        LoadedImageFromFile& cur_ceiling_tex
    );

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

    Cell charToCell(char c, r32& orientation) const;
    char cellToChar(Cell cell, r32 orientation) const;

private:
    std::vector<std::vector<Cell>> cells;
};

#endif
