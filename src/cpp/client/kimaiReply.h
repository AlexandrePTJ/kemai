#pragma once

#include <atomic>
#include <optional>

#include <QObject>
#include <QString>

namespace kemai {

/*!
 * API Request result base class
 * This class is just here because of Q_OBJECT limitation : This macro can't be used in template class
 * So we define a base class that just defines the signal that we want to emit in the templated class
 */
class KimaiApiBaseResult : public QObject
{
    Q_OBJECT

public:
    /*!
     * Mark the request result as ready
     * It will emit the ready finished
     */
    void markAsReady();

    bool hasError() const;

    /*!
     * Mark the request result as ready
     * It will emit the ready finished
     */
    void setError(const QString& errorMessage);

    QString errorMessage() const;

signals:
    void ready();
    void error();

private:
    std::atomic<bool> mIsReady = false;
    std::optional<QString> mError;
};

template<class ResultType> class KimaiApiResult : public KimaiApiBaseResult
{
public:
    /*!
     * Return the wrapped result
     * Note that the return result is undefined before ready signal receiving
     * \return						A reference on the request result
     */
    const ResultType& getResult() const { return m_result; }

    /*!
     * Set the request result
     * Calling this method lead to the ready signal emission
     * \param[in]	result   		Reference on the request result
     */
    void setResult(const ResultType& result)
    {
        m_result = result;

        this->markAsReady();
    }

    /*!
     * Set the request result (move version)
     * Calling this method lead to the ready signal emission
     * \param[in]	result   		Reference on the request result
     */
    void setResult(ResultType&& result)
    {
        m_result = std::move(result);

        this->markAsReady();
    }

    template<class Q = ResultType>
    typename std::enable_if<std::is_move_constructible<ResultType>::value, Q>::type takeResult()
    {
        return std::move(m_result);
    }

private:
    ResultType m_result;
};

} // namespace kemai
