#pragma once

#include "PriceLevel.hpp"
#include <algorithm>

class AVLTree {
    PriceLevel* root_ = nullptr;
    PriceLevel* min_  = nullptr;
    PriceLevel* max_  = nullptr;

    static int height(PriceLevel* n) { return n ? n->height : 0; }

    static int balanceFactor(PriceLevel* n) {
        return height(n->left) - height(n->right);
    }

    static void updateHeight(PriceLevel* n) {
        n->height = 1 + std::max(height(n->left), height(n->right));
    }

    PriceLevel* rotateRight(PriceLevel* y) {
        PriceLevel* x = y->left;
        y->left  = x->right;
        x->right = y;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    PriceLevel* rotateLeft(PriceLevel* x) {
        PriceLevel* y = x->right;
        x->right = y->left;
        y->left  = x;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    PriceLevel* balance(PriceLevel* node) {
        updateHeight(node);
        int bf = balanceFactor(node);

        if (bf > 1) {
            if (balanceFactor(node->left) < 0)
                node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (bf < -1) {
            if (balanceFactor(node->right) > 0)
                node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    PriceLevel* insertNode(PriceLevel* node, PriceLevel* newNode) {
        if (!node) return newNode;

        if (newNode->price < node->price)
            node->left = insertNode(node->left, newNode);
        else
            node->right = insertNode(node->right, newNode);

        return balance(node);
    }

    PriceLevel* removeNode(PriceLevel* node, int32_t price) {
        if (!node) return nullptr;

        if (price < node->price) {
            node->left = removeNode(node->left, price);
        } else if (price > node->price) {
            node->right = removeNode(node->right, price);
        } else {
            if (!node->left) {
                PriceLevel* right = node->right;
                node->left = node->right = nullptr;
                return right;
            }
            if (!node->right) {
                PriceLevel* left = node->left;
                node->left = node->right = nullptr;
                return left;
            }

            PriceLevel* successor = node->right;
            while (successor->left) successor = successor->left;

            node->right = removeNode(node->right, successor->price);

            successor->left  = node->left;
            successor->right = node->right;
            node->left = node->right = nullptr;

            return balance(successor);
        }

        return balance(node);
    }

public:
    void insert(PriceLevel* level) {
        root_ = insertNode(root_, level);
        if (!min_ || level->price < min_->price) min_ = level;
        if (!max_ || level->price > max_->price) max_ = level;
    }

    void remove(PriceLevel* level) {
        bool wasMin = (level == min_);
        bool wasMax = (level == max_);

        root_ = removeNode(root_, level->price);

        if (wasMin) {
            min_ = root_;
            if (min_) while (min_->left) min_ = min_->left;
        }
        if (wasMax) {
            max_ = root_;
            if (max_) while (max_->right) max_ = max_->right;
        }
    }

    PriceLevel* min()  const { return min_; }
    PriceLevel* max()  const { return max_; }
    bool        empty() const { return root_ == nullptr; }
};
