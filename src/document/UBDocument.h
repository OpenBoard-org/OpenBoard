/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include <QList>

#include <memory>


// forward
class UBDocumentProxy;
class UBGraphicsScene;
class UBThumbnailScene;
class UBToc;


/**
 * @brief The UBDocument class represents a document.
 *
 * In contrast to the UBDocumentProxy it is not only a pointer to the document directory,
 * but manages the document thumbnails and provides the functions to add, delete and move
 * pages.
 *
 * Instances of this class can be retrieved by providing a UBDocumentProxy to the static
 * getDocument() function. They are always referenced by a shared pointer and live as long
 * as anybody wants to work with the doument.
 */
class UBDocument
{
private:
    UBDocument(std::shared_ptr<UBDocumentProxy> proxy);

public:
    ~UBDocument();

    std::shared_ptr<UBDocumentProxy> proxy() const;

    void deletePages(QList<int> indexes);
    void duplicatePage(int index);
    void movePage(int fromIndex, int toIndex);
    void copyPage(int fromIndex, std::shared_ptr<UBDocument> to, int toIndex);
    std::shared_ptr<UBGraphicsScene> createPage(int index, bool useUndoRedoStack = true);
    void persistPage(std::shared_ptr<UBGraphicsScene> scene, int index, bool isAutomaticBackup = false,
                     bool forceImmediateSaving = false);
    std::shared_ptr<UBGraphicsScene> loadScene(int index, bool cacheNeighboringScenes = true);
    std::shared_ptr<UBGraphicsScene> getScene(int index);
    QList<QString> pageRelativeDependencies(int index);
    UBThumbnailScene* thumbnailScene() const;
    UBToc* toc();

    int pageCount();

    QString sceneFile(int index);
    QString thumbnailFile(int index);

    static std::shared_ptr<UBDocument> getDocument(std::shared_ptr<UBDocumentProxy> proxy);

private:
    void scan();
    void copyPage(int fromIndex, UBDocument* to, int toIndex);

    static std::shared_ptr<UBDocument> findDocument(std::shared_ptr<UBDocumentProxy> proxy);

private:
    std::shared_ptr<UBDocumentProxy> mProxy{nullptr};
    UBThumbnailScene* mThumbnailScene{nullptr};
    UBToc* mToc{nullptr};

    static QList<std::weak_ptr<UBDocument>> sDocuments;
};
