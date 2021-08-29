#pragma once
#include <hex.hpp>

#include <hex/data_processor/attribute.hpp>

#include <set>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace hex::prv { class Provider; class Overlay; }

namespace hex::dp {

    class Node {
    public:
        Node(std::string_view unlocalizedTitle, std::vector<Attribute> attributes);

        virtual ~Node() = default;

        [[nodiscard]] u32 getID() const { return this->m_id; }
        void setID(u32 id) { this->m_id = id; }

        [[nodiscard]] std::string_view getUnlocalizedName() const { return this->m_unlocalizedName; }
        void setUnlocalizedName(std::string_view unlocalizedName) { this->m_unlocalizedName = unlocalizedName; }

        [[nodiscard]] std::string_view getUnlocalizedTitle() const { return this->m_unlocalizedTitle; }
        [[nodiscard]] std::vector<Attribute>& getAttributes() { return this->m_attributes; }

        void setCurrentOverlay(prv::Overlay *overlay) {
            this->m_overlay = overlay;
        }

        virtual void drawNode() { }
        virtual void process() = 0;

        virtual void store(nlohmann::json &j) { }
        virtual void load(nlohmann::json &j) { }

        using NodeError = std::pair<Node*, std::string>;

        void resetOutputData() {
            for (auto &attribute : this->m_attributes)
                attribute.getOutputData().reset();
        }

        void resetProcessedInputs() {
            this->m_processedInputs.clear();
        }

    private:
        u32 m_id;
        std::string m_unlocalizedTitle, m_unlocalizedName;
        std::vector<Attribute> m_attributes;
        std::set<u32> m_processedInputs;
        prv::Overlay *m_overlay = nullptr;

        Attribute* getConnectedInputAttribute(u32 index) {
            if (index >= this->getAttributes().size())
                throw std::runtime_error("Attribute index out of bounds!");

            auto &connectedAttribute = this->getAttributes()[index].getConnectedAttributes();

            if (connectedAttribute.empty())
                return nullptr;

            return connectedAttribute.begin()->second;
        }

        void markInputProcessed(u32 index) {
            const auto &[iter, inserted] = this->m_processedInputs.insert(index);
            if (!inserted)
                throwNodeError("Recursion detected!");
        }

    protected:

        [[noreturn]] void throwNodeError(std::string_view message) {
            throw NodeError(this, message);
        }

        std::vector<u8> getBufferOnInput(u32 index);
        u64 getIntegerOnInput(u32 index);
        float getFloatOnInput(u32 index);

        void setBufferOnOutput(u32 index, std::vector<u8> data);
        void setIntegerOnOutput(u32 index, u64 integer);
        void setFloatOnOutput(u32 index, float floatingPoint);

        void setOverlayData(u64 address, const std::vector<u8> &data);

    };

}