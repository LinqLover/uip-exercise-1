#include <QTextStream>


// CREDITS: https://stackoverflow.com/a/25506213/13994294 (edited)
QStringList intersection(const QList<QStringList> &vecs) {
    auto last_intersection = vecs[0];
    QStringList curr_intersection;
    for (auto i = 1; i < vecs.size(); ++i) {
        std::set_intersection(last_intersection.begin(), last_intersection.end(),
            vecs[i].begin(), vecs[i].end(),
            std::back_inserter(curr_intersection));
        std::swap(last_intersection, curr_intersection);
        curr_intersection.clear();
    }
    return last_intersection;
}

// TODO: Support more types (reinvention of qdebug)
QString variantToString(const QVariant variant) {
    if (variant.userType() == QMetaType::QString) {
        return '"' + variant.toString() + '"';
    }
    if (variant.canConvert(QMetaType::QVariantList)) {
        const auto list = variant.toList();
        QStringList strings;
        strings.reserve(list.size());
        std::transform(list.begin(), list.end(), std::back_inserter(strings), variantToString);
        return "{" + strings.join(", ") + "}";
    }
    if (variant.userType() == QMetaType::QRegExp) {
        return "QRegExp(" + variantToString(variant.toRegExp().pattern()) + ")";
    }
    return variant.toString();
}
