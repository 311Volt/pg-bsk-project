
#ifndef PG_BSK_PROJECT_FILE_SENDER_HPP
#define PG_BSK_PROJECT_FILE_SENDER_HPP

#include <memory>
#include <map>
#include <set>

class FileSender : public std::enable_shared_from_this<FileSender> {
public:
	
	static std::shared_ptr<FileSender> StartSendingFile(std::string filePath,
			ENCRYPTION_MODE encryptionMode);
	
	
	~FileSender();
	
	float GetProgress() const;
	size_t PGetSize() const;
	void GetSha256(uint8_t* hash) const;
	std::string GetFileName() const;
	bool IsDone() const;
	
private:
	
	FileSender();
	
	std::vector<uint8_t> sha512;
	bool done;
	std::set<uint64_t> sentBlocks, pendingBlocks, queueBlocks;
};

#endif

