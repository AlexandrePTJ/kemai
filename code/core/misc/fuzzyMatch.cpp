// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "fuzzyMatch.h"

// Qt headers
#include <QStringList>

namespace kemai
{
    bool FuzzyMatch::wordPrefixMatch(const QString &query, const QString &entry)
    {
        if (query.isEmpty())
        {
            return true;
        }

        const auto q = query.toLower();
        const auto e = entry.toLower();

        QStringList words;
        QString     current;
        for (const QChar ch : e)
        {
            if (ch.isLetterOrNumber())
            {
                current.append(ch);
            }
            else if (!current.isEmpty())
            {
                words.append(current);
                current.clear();
            }
        }
        if (!current.isEmpty())
        {
            words.append(current);
        }

        int qi = 0;
        int wi = 0;
        int ci = 0;
        while (qi < q.size() && wi < words.size())
        {
            const QChar qc = q[qi];
            if (!qc.isLetterOrNumber())
            {
                // Any non-alphanumeric (space, dash, parenthesis...) is a word
                // boundary in the query, mirroring how the entry was split.
                if (ci > 0)
                {
                    wi++;
                    ci = 0;
                }
                qi++;
            }
            else if (ci < words[wi].size() && words[wi][ci] == qc)
            {
                qi++;
                ci++;
            }
            else if (ci == 0)
            {
                // First char of this word doesn't match: skip to next word.
                wi++;
            }
            else
            {
                // Mid-word mismatch: restart matching from next word.
                wi++;
                ci = 0;
            }
        }

        while (qi < q.size() && !q[qi].isLetterOrNumber())
        {
            qi++;
        }
        return qi == q.size();
    }
} // namespace kemai
